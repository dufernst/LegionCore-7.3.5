////////////////////////////////////////////////////////////////////////////////
//
//  MILLENIUM-STUDIO
//  Copyright 2016 Millenium-studio SARL
//  All Rights Reserved.
//
////////////////////////////////////////////////////////////////////////////////

#include "highmaul.hpp"

G3D::Vector3 creepingMossPo[MaxCreepingMoss] =
{
    {4096.283f, 7719.135f, 0.2535536f},
    {4107.993f, 7719.417f, 0.2534866f},
    {4100.202f, 7735.637f, 0.2534722f},
    {4095.731f, 7745.500f, 0.2534722f},
    {4105.304f, 7757.403f, 0.2534722f},
    {4124.274f, 7726.856f, 0.2535520f},
    {4114.686f, 7764.564f, 0.2534722f},
    {4128.087f, 7739.741f, 0.2534592f},
    {4138.525f, 7750.965f, 0.2534567f},
    {4148.499f, 7756.370f, 0.2527755f},
    {4111.810f, 7739.060f, 0.2535081f},
    {4133.647f, 7758.851f, 0.2534722f},
    {4126.690f, 7767.401f, 0.2534722f},
    {4139.254f, 7735.781f, 0.2535115f},
    {4118.909f, 7753.566f, 0.2534722f},
    {4151.049f, 7743.521f, 0.6013964f}
};

Position const fleshEaterSpawns[MaxFleshEaterPos] =
{
    {4178.046f, 7791.621f, -0.3537667f, 3.052145f},
    {4141.948f, 7720.839f, -1.1697650f, 2.001688f}
};

G3D::Vector3 beachCenter = {4104.36f, 7769.18f, 0.254f};

void ResetPlayersPower(Creature* source)
{
    if (source == nullptr)
        return;

    source->GetMap()->ApplyOnEveryPlayer([&](Player* player)
    {
        if (!player)
            return;

        player->SetPower(POWER_ALTERNATE, 0);
    });
}

/// Brackenspore <Walker of the Deep> - 78491
struct boss_brackenspore : public BossAI
{
    enum eSpells
    {
        /// Misc
        RotDot = 163241,
        CreepingMossPeriodic = 163347,
        CreepingMossAreaTrigger = 173229,
        FlamethrowerDespawnAT = 173281,
        BFC9000 = 164175,
        FlamethrowerAura = 163663,
        BurningInfusion = 165223,
        EnergyRegen = 164248,
        /// Necrotic Breath
        SpellNecroticBreath = 159219,
        SpellInfestingSpores = 159996,
        /// Mind Fungus
        SummonMindFungus = 163141,
        /// Fungal Flesh-Eater
        SummonFungalFleshEater = 163142,
        /// Spore Shooter
        SporeShooterDummy = 163594,
        SummonSporeShooter = 160446,
        /// Living Mushroom
        SummonLivingMushroom = 160022,
        /// Rejuvenating Mushroom
        RejuvenatingMushDummy = 177820,
        SummonRejuvenatingMush = 160021,
        /// Loot
        BrackensporeBonus = 177524,

        /// Mythic mode only
        SpellCallOfTheTides = 160425,
        CallOfTheTidesSummonAT = 160413
    };

    enum eEvents
    {
        EventNecroticBreath = 1,
        EventBerserker,
        EventInfestingSpores,
        EventCheckForIntro,
        EventMindFungus,
        EventLivingMushroom,
        EventSporeShooter,
        EventFungalFleshEater,
        EventRejuvenatingMushroom,
        EventSpecialAbility,
        EventScheduleEnergy,
        EventRot
    };

    enum eActions
    {
        DoIntro,
        CreepingMoss,
        InfestingSpores
    };

    enum eCreatures
    {
        /// Cosmetic part
        IronWarmaster = 86609,
        IronFlameTechnician = 86607,
        MindFungus = 86611,
        SporeShooter = 86612,
        WorldTrigger = 59481,
        /// Fight
        SporeShooterFight = 79183,
        MindFungusFight = 79082,
        FungalFleshEater = 79092,
        LivingMushroom = 78884,
        RejuvenatingMush = 78868,
        InvisibleMan = 64693
    };

    enum eTalk
    {
        WarnInfestingSpores
    };

    boss_brackenspore(Creature* creature) : BossAI(creature, BossBrackenspore)
    {
        m_Instance = creature->GetInstanceScript();
        m_IntroDone = false;
    }

    EventMap m_Events;
    InstanceScript* m_Instance;
    EventMap m_CosmeticEvent;
    std::list<ObjectGuid> m_Creatures;
    bool m_IntroDone;
    std::vector<ObjectGuid> m_Flamethrowers;

    void Reset() override
    {
        m_Events.Reset();

        _Reset();

        me->RemoveAura(Berserker);
        me->RemoveAura(CreepingMossPeriodic);

        me->SetPower(POWER_RAGE, 0);
        me->SetMaxPower(POWER_RAGE, 500);

        me->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_REGENERATE_POWER);

        me->RemoveAura(EnergyRegen);

        m_Instance->DoRemoveAurasDueToSpellOnPlayers(RotDot);
        m_Instance->DoRemoveAurasDueToSpellOnPlayers(FlamethrowerAura);
        m_Instance->DoRemoveAurasDueToSpellOnPlayers(BurningInfusion);
        m_Instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

        ResetPlayersPower(me);

        if (!m_IntroDone)
        {
            me->SetUInt32Value(UNIT_FIELD_EMOTE_STATE, 438);

            AddDelayedEvent(1 * IN_MILLISECONDS, [this]() -> void
            {
                for (uint8 i = 0; i < MaxCreepingMoss; ++i)
                    me->CastSpell(creepingMossPo[i], CreepingMossAreaTrigger, true);
            });
        }
        else
        {
            me->SetUInt32Value(UNIT_FIELD_EMOTE_STATE, 0);

            if (m_Flamethrowers.empty())
            {
                std::list<Creature*> bfcs;
                me->GetCreatureListWithEntryInGrid(bfcs, BFC9000, 100.0f);

                for (Creature* creature : bfcs)
                {
                    creature->Respawn(true);
                    creature->CastSpell(creature, BFC9000, true);

                    if (creature->AI())
                        creature->AI()->Reset();

                    m_Flamethrowers.push_back(creature->GetGUID());
                }
            }
            else
            {
                for (auto guid : m_Flamethrowers)
                {
                    if (Creature* creature = Creature::GetCreature(*me, guid))
                    {
                        creature->Respawn(true);
                        creature->CastSpell(creature, BFC9000, true);

                        if (creature->AI())
                            creature->AI()->Reset();
                    }
                }
            }
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
            case DoIntro:
            {
                m_IntroDone = true;

                std::list<Creature*> warmasters;
                me->GetCreatureListWithEntryInGrid(warmasters, IronWarmaster, 50.0f);

                if (!warmasters.empty())
                    warmasters.sort(Trinity::ObjectDistanceOrderPred(me, false));

                if (Creature* ironWar = (*warmasters.begin()))
                {
                    if (ironWar->GetAI())
                        ironWar->AI()->DoAction(DoIntro);

                    warmasters.remove(ironWar);
                }

                AddDelayedEvent(5 * IN_MILLISECONDS, [this]() -> void
                {
                    for (ObjectGuid guid : m_Flamethrowers)
                        if (Creature* creature = Creature::GetCreature(*me, guid))
                            creature->CastSpell(creature, BFC9000, true);

                    std::list<Creature*> mindFungus;
                    me->GetCreatureListWithEntryInGrid(mindFungus, MindFungus, 100.0f);

                    for (Creature* creature : mindFungus)
                        creature->DespawnOrUnsummon();

                    if (Creature* shooter = me->FindNearestCreature(SporeShooter, 100.0f))
                        shooter->DespawnOrUnsummon();

                    std::list<Creature*> ironFlames;
                    me->GetCreatureListWithEntryInGrid(ironFlames, IronFlameTechnician, 50.0f);

                    for (Creature* creature : ironFlames)
                    {
                        if (creature->IsAIEnabled)
                            creature->AI()->DoAction(DoIntro);

                        if (Creature* trigger = creature->FindNearestCreature(WorldTrigger, 100.0f))
                        {
                            trigger->SetReactState(REACT_PASSIVE);
                            trigger->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);

                            creature->InterruptNonMeleeSpells(true);
                            creature->GetMotionMaster()->MovePoint(0, *trigger);
                        }
                    }
                });

                std::list<ObjectGuid> warMasterGuids;
                for (Creature* itr : warmasters)
                    warMasterGuids.push_back(itr->GetGUID());

                AddDelayedEvent(8 * IN_MILLISECONDS, [this, warMasterGuids]() -> void
                {
                    if (warMasterGuids.empty())
                        return;

                    if (Creature* ironWar = Creature::GetCreature(*me, (*warMasterGuids.begin())))
                        me->Kill(ironWar);

                    Reset();
                });

                break;
            }
            case CreepingMoss:
            {
                SummonCreepingMoss();
                break;
            }
            case InfestingSpores:
            {
                m_Events.RescheduleEvent(EventInfestingSpores, 1);
                break;
            }
            default:
                break;
        }
    }

    void EnterCombat(Unit* /*attacker*/) override
    {
        _EnterCombat();

        m_Events.RescheduleEvent(EventNecroticBreath, 30 * IN_MILLISECONDS);
        m_Events.RescheduleEvent(EventBerserker, 600 * IN_MILLISECONDS);
        m_Events.RescheduleEvent(EventMindFungus, 10 * IN_MILLISECONDS);
        m_Events.RescheduleEvent(EventLivingMushroom, 17 * IN_MILLISECONDS);
        m_Events.RescheduleEvent(EventSporeShooter, 20 * IN_MILLISECONDS);
        m_Events.RescheduleEvent(EventFungalFleshEater, 32 * IN_MILLISECONDS);
        m_Events.RescheduleEvent(EventRejuvenatingMushroom, 80 * IN_MILLISECONDS);
        m_Events.RescheduleEvent(EventRot, 10 * IN_MILLISECONDS);

        /// Mythic Specials. Shared cd, which special he uses is random.
        if (IsMythicRaid())
            m_Events.RescheduleEvent(EventSpecialAbility, 20 * IN_MILLISECONDS);

        m_Instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me, 1);

        /// Spawn timer for Creeping Moss AreaTrigger
        /// 5s for LFR, 2s for Normal mode, 1.85s for Heroic mode
        /// 1.4s for Mythic mode and 1.75s for others
        me->CastSpell(me, CreepingMossPeriodic, true);
        me->CastSpell(me, EnergyRegen, true);

        std::list<Creature*> cosmeticMobs;
        me->GetCreatureListWithEntryInGrid(cosmeticMobs, BlackrockGrunt, 800.0f);

        for (Creature* grunt : cosmeticMobs)
            grunt->DespawnOrUnsummon();

        cosmeticMobs.clear();
        me->GetCreatureListWithEntryInGrid(cosmeticMobs, IronFlameTechnician, 80.0f);

        for (Creature* technician : cosmeticMobs)
            technician->DespawnOrUnsummon();

        cosmeticMobs.clear();
        me->GetCreatureListWithEntryInGrid(cosmeticMobs, IronWarmaster, 80.0f);

        for (Creature* warmaster : cosmeticMobs)
            warmaster->DespawnOrUnsummon();
    }

    void JustDied(Unit* /*killer*/) override
    {
        _JustDied();

        m_Instance->DoRemoveAurasDueToSpellOnPlayers(RotDot);
        m_Instance->DoRemoveAurasDueToSpellOnPlayers(FlamethrowerAura);
        m_Instance->DoRemoveAurasDueToSpellOnPlayers(BurningInfusion);

        m_Instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

        CastSpellToPlayers(me->GetMap(), me, BrackensporeBonus, true);

        ResetPlayersPower(me);

        for (ObjectGuid guid : m_Flamethrowers)
            if (Creature* creature = Creature::GetCreature(*me, guid))
                creature->RemoveAura(BFC9000);
    }

    void EnterEvadeMode() override
    {
        CreatureAI::EnterEvadeMode();

        m_Instance->SetBossState(BossBrackenspore, FAIL);
    }

    void SetGUID(ObjectGuid const& guid, int32 /*id*/) override
    {
        m_Creatures.push_back(guid);
        m_CosmeticEvent.RescheduleEvent(EventCheckForIntro, 1000);
    }

    void SpellHitTarget(Unit* target, SpellInfo const* spellInfo) override
    {
        if (target == nullptr)
            return;

        switch (spellInfo->Id)
        {
            case SummonMindFungus:
                me->SummonCreature(MindFungusFight, *target, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 20 * IN_MILLISECONDS);
                break;
            case SummonFungalFleshEater:
                me->SummonCreature(FungalFleshEater, fleshEaterSpawns[urand(1, MaxFleshEaterPos) - 1], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 20 * IN_MILLISECONDS);
                break;
            case SporeShooterDummy:
            {
                uint8 count = IsMythicRaid() ? 4 : 2;
                for (uint8 i = 0; i < count; ++i)
                    me->CastSpell(me, SummonSporeShooter, true);
                break;
            }
            case RejuvenatingMushDummy:
                me->CastSpell(target, SummonRejuvenatingMush, true);
                break;
            case SpellCallOfTheTides:
                target->CastSpell(target, CallOfTheTidesSummonAT, true, nullptr, nullptr, me->GetGUID());
                break;
            default:
                break;
        }
    }

    void RegeneratePower(Powers /*power*/, float& value) override
    {
        /// Brackenspore only regens by script
        value = 0;
    }

    void UpdateAI(uint32 diff) override
    {
        m_CosmeticEvent.Update(diff);

        if (m_CosmeticEvent.ExecuteEvent())
        {
            bool canSchedule = true;
            for (ObjectGuid guid : m_Creatures)
            {
                if (Creature* add = Creature::GetCreature(*me, guid))
                {
                    if (add->isAlive())
                    {
                        canSchedule = false;
                        break;
                    }
                }
            }

            if (canSchedule)
                DoAction(DoIntro);
            else
                m_CosmeticEvent.RescheduleEvent(EventCheckForIntro, 1000);
        }

        if (!UpdateVictim())
            return;

        m_Events.Update(diff);

        /// Update moves here, avoid some movements problems during Infesting Spores
        if (me->getVictim() && !me->IsWithinMeleeRange(me->getVictim()) && me->HasAura(SpellInfestingSpores))
        {
            Position pos;
            me->getVictim()->GetPosition(&pos);

            me->GetMotionMaster()->Clear();
            me->GetMotionMaster()->MovePoint(0, pos);
        }

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        switch (m_Events.ExecuteEvent())
        {
            case EventNecroticBreath:
            {
                me->CastSpell(me, SpellNecroticBreath, false);
                m_Events.RescheduleEvent(EventNecroticBreath, 32 * IN_MILLISECONDS);
                break;
            }
            case EventBerserker:
            {
                me->CastSpell(me, Berserker, true);
                break;
            }
            case EventInfestingSpores:
            {
                Talk(WarnInfestingSpores);
                me->RemoveAura(EnergyRegen);
                me->CastSpell(me, SpellInfestingSpores, false);
                m_Events.RescheduleEvent(EventScheduleEnergy, 12 * IN_MILLISECONDS);
                break;
            }
            case EventMindFungus:
            {
                me->CastSpell(me, SummonMindFungus, true);
                m_Events.RescheduleEvent(EventMindFungus, 30 * IN_MILLISECONDS);
                break;
            }
            case EventLivingMushroom:
            {
                me->CastSpell(me, SummonLivingMushroom, true);
                m_Events.RescheduleEvent(EventLivingMushroom, 55 * IN_MILLISECONDS);
                break;
            }
            case EventSporeShooter:
            {
                me->CastSpell(me, SporeShooterDummy, true);
                m_Events.RescheduleEvent(EventSporeShooter, 57 * IN_MILLISECONDS);
                break;
            }
            case EventFungalFleshEater:
            {
                me->CastSpell(me, SummonFungalFleshEater, true);
                m_Events.RescheduleEvent(EventFungalFleshEater, 120 * IN_MILLISECONDS);
                break;
            }
            case EventRejuvenatingMushroom:
            {
                me->CastSpell(me, RejuvenatingMushDummy, true);
                m_Events.RescheduleEvent(EventRejuvenatingMushroom, 130 * IN_MILLISECONDS);
                break;
            }
            case EventSpecialAbility:
            {
                DoSpecialAbility();
                m_Events.RescheduleEvent(EventSpecialAbility, 20 * IN_MILLISECONDS);
                break;
            }
            case EventScheduleEnergy:
            {
                me->GetMotionMaster()->Clear();

                if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                    me->GetMotionMaster()->MoveChase(target);

                me->CastSpell(me, EnergyRegen, true);
                break;
            }
            case EventRot:
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                    me->CastSpell(target, RotDot, true);
                m_Events.RescheduleEvent(EventRot, 10 * IN_MILLISECONDS);
                break;
            }
            default:
                break;
        }

        /// If boss is outside the beach, it should resets
        if (me->GetDistance(beachCenter.x, beachCenter.y, beachCenter.z) >= 100.0f)
        {
            EnterEvadeMode();
            return;
        }

        DoMeleeAttackIfReady();
    }

    void SummonCreepingMoss()
    {
        Map* map = me->GetMap();
        if (map == nullptr)
            return;

        float o = frand(0.0f, 2 * M_PI);

        /// Use different spawn radius depending on orientation
        float radius = GetSpawnRangeByOrientation(o);

        float oStep = 2 * M_PI / 30.0f;
        float x = beachCenter.x + (radius * cos(o));
        float y = beachCenter.y + (radius * sin(o));
        float z = map->GetHeight(x, y, MAX_HEIGHT);

        /// First of all, verify if we can spawn an AreaTrigger all around the beach center
        /// With a radius of 60-80 yards, it must cover the beach progressively
        if (me->IsWithinLOS(x, y, z) && CheckCreepingMossPosition(x, y))
        {
            me->CastSpell(x, y, z, CreepingMossAreaTrigger, true);
            return;
        }

        for (uint8 j = 0; j < 30; ++j)
        {
            o = Position::NormalizeOrientation(o + oStep);
            radius = GetSpawnRangeByOrientation(o);

            x = beachCenter.x + (radius * cos(o));
            y = beachCenter.y + (radius * sin(o));
            z = map->GetHeight(x, y, MAX_HEIGHT);

            if (me->IsWithinLOS(x, y, z) && CheckCreepingMossPosition(x, y))
            {
                me->CastSpell(x, y, z, CreepingMossAreaTrigger, true);
                return;
            }
        }

        float maxRadius = 80.0f;
        float step = 2.0f;

        /// Secondly, check for each less radius (2 yard step) ...
        for (uint8 i = 0; i < 45; ++i)
        {
            maxRadius -= step;
            radius = GetSpawnRangeByOrientation(o, maxRadius);

            x = beachCenter.x + (radius * cos(o));
            y = beachCenter.y + (radius * sin(o));
            z = map->GetHeight(x, y, MAX_HEIGHT);

            if (me->IsWithinLOS(x, y, z) && CheckCreepingMossPosition(x, y))
            {
                me->CastSpell(x, y, z, CreepingMossAreaTrigger, true);
                return;
            }

            /// ... and for each orientation (3.33% step)
            for (uint8 j = 0; j < 30; ++j)
            {
                o = Position::NormalizeOrientation(o + oStep);
                radius = GetSpawnRangeByOrientation(o, maxRadius);

                x = beachCenter.x + (radius * cos(o));
                y = beachCenter.y + (radius * sin(o));
                z = map->GetHeight(x, y, MAX_HEIGHT);

                if (me->IsWithinLOS(x, y, z) && CheckCreepingMossPosition(x, y))
                {
                    me->CastSpell(x, y, z, CreepingMossAreaTrigger, true);
                    return;
                }
            }
        }
    }

    bool CheckCreepingMossPosition(float x, float y) const
    {
        return true;
    }

    float GetSpawnRangeByOrientation(float orientation, float maxRadius = 80.0f) const
    {
        if (orientation <= (M_PI / 4.0f))
            return maxRadius;
        else if (orientation <= (3 * M_PI / 4.0f))
            return maxRadius - 20.0f;
        else if (orientation <= (5 * M_PI / 4.0f))
            return maxRadius;
        else if (orientation <= (2 * M_PI - M_PI / 4))
            return maxRadius - 20.0f;
        else
            return maxRadius;
    }

    void DoSpecialAbility()
    {
        /// Call of the Tides
        if (urand(0, 1))
            me->CastSpell(me, SpellCallOfTheTides, true);
    }
};

/// Mind Fungus (Cosmetic) - 86611
/// Mind Fungus (For fight) - 79082
struct npc_highmaul_mind_fungus : public ScriptedAI
{
    enum eSpells
    {
        MindFungusVisual = 171862,
        MindFungusAura = 163138,
        MindFungusAT = 159489
    };

    npc_highmaul_mind_fungus(Creature* creature) : ScriptedAI(creature) { }

    void Reset() override
    {
        me->CastSpell(me, MindFungusVisual, true);
        me->CastSpell(me, MindFungusAura, true);
        me->CastSpell(me, MindFungusAT, true);

        me->AddUnitState(UNIT_STATE_STUNNED);
    }

    void JustDied(Unit* /*killer*/) override
    {
        me->RemoveAura(MindFungusVisual);
        me->RemoveAura(MindFungusAura);
        me->RemoveAura(MindFungusAT);
        me->DespawnOrUnsummon(500);
    }

    void UpdateAI(uint32 /*diff*/) override { }
};


/// Spore Shooter (Cosmetic) - 86612
/// Spore Shooter (For fight) - 79183
struct npc_highmaul_spore_shooter : public ScriptedAI
{
    enum eSpell
    {
        SporeShot = 173241
    };

    enum eEvent
    {
        EventSporeShot = 1
    };

    npc_highmaul_spore_shooter(Creature* creature) : ScriptedAI(creature) { }

    EventMap m_Events;

    void Reset() override
    {
        me->AddUnitState(UNIT_STATE_ROOT);

        m_Events.Reset();
    }

    void EnterCombat(Unit* /*attacker*/) override
    {
        m_Events.RescheduleEvent(EventSporeShot, urand(100, 1500));
    }

    void JustDied(Unit* /*killer*/) override
    {
        me->DespawnOrUnsummon(20 * IN_MILLISECONDS);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        m_Events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        if (m_Events.ExecuteEvent() == EventSporeShot)
        {
            me->CastSpell(me, SporeShot, false);
            m_Events.RescheduleEvent(EventSporeShot, 4500);
        }
    }
};

/// Fungal Flesh-Eater - 79092
struct npc_highmaul_fungal_flesh_eater : public MS::AI::CosmeticAI
{
    enum eTalk
    {
        Warn
    };

    enum eSpells
    {
        InfestedWaters = 164644,
        FleshEater = 159972,
        Decay = 160013
    };

    enum eEvents
    {
        EventDecay = 1,
        EventFleshEater
    };

    enum eData
    {
        AnimKit = 1718
    };

    npc_highmaul_fungal_flesh_eater(Creature* creature) : MS::AI::CosmeticAI(creature)
    {
        m_Instance = creature->GetInstanceScript();
    }

    InstanceScript* m_Instance;
    EventMap m_Events;
    bool m_Scheduled;

    void Reset() override
    {
        me->AddUnitState(UNIT_STATE_ROOT);

        Talk(Warn);

        AddDelayedEvent(1 * IN_MILLISECONDS, [this]() -> void
        {
            me->SetFloatValue(OBJECT_FIELD_SCALE, 1.0f);
            me->SetFloatValue(UNIT_FIELD_BOUNDING_RADIUS, 0.31f);
            me->SetFloatValue(UNIT_FIELD_COMBAT_REACH, 7.0f);
        });

        //AddDelayedEvent(5 * IN_MILLISECONDS, [this]() -> void { me->PlayOneShotAnimKitAnimKit); });
        AddDelayedEvent(6 * IN_MILLISECONDS, [this]() -> void { me->ClearUnitState(UNIT_STATE_ROOT); });

        m_Instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me, 2);

        m_Scheduled = false;
    }

    void DamageDealt(Unit* /*victim*/, uint32& /*damage*/, DamageEffectType /*damageType*/) override
    {
        if (!m_Scheduled)
        {
            m_Scheduled = true;
            m_Events.RescheduleEvent(EventDecay, 5 * IN_MILLISECONDS);
            m_Events.RescheduleEvent(EventFleshEater, 1 * IN_MILLISECONDS);
        }
    }

    void JustDied(Unit* /*killer*/) override
    {
        m_Instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
    }

    void UpdateAI(uint32 diff) override
    {
        MS::AI::CosmeticAI::UpdateAI(diff);

        if (me->GetPositionZ() <= 0.0f && !me->HasAura(InfestedWaters))
            me->CastSpell(me, InfestedWaters, true);
        else if (me->GetPositionZ() > 0.0f && me->HasAura(InfestedWaters))
            me->RemoveAura(InfestedWaters);

        if (!UpdateVictim())
            return;

        m_Events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        switch (m_Events.ExecuteEvent())
        {
            case EventDecay:
                me->CastSpell(me, Decay, false);
                m_Events.RescheduleEvent(EventDecay, 10 * IN_MILLISECONDS);
                break;
            case EventFleshEater:
                me->CastSpell(me, FleshEater, true);
                m_Events.RescheduleEvent(EventFleshEater, 10 * IN_MILLISECONDS);
                break;
            default:
                break;
        }

        DoMeleeAttackIfReady();
    }
};

/// Living Mushroom - 78884
struct npc_highmaul_living_mushroom : public MS::AI::CosmeticAI
{
    enum eSpells
    {
        LivingMushroomVisual = 159280,
        LivingMushroomL1Visual = 164245,
        Withering3Percent = 160399,
        Withering5Percent = 163113,
        LivingSpores = 159291
    };

    npc_highmaul_living_mushroom(Creature* creature) : MS::AI::CosmeticAI(creature)
    {
        m_Instance = creature->GetInstanceScript();
    }

    InstanceScript* m_Instance;

    void Reset() override
    {
        me->setRegeneratingHealth(false);
        me->SetHealth(me->GetMaxHealth() / 2);
        me->AddUnitState(UNIT_STATE_STUNNED);

        me->CastSpell(me, LivingMushroomVisual, true);
        me->CastSpell(me, LivingMushroomL1Visual, true);

        m_Instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me, 3);

        AddDelayedEvent(6 * IN_MILLISECONDS, [this]() -> void { me->CastSpell(me, Withering3Percent, true); });

        AddDelayedEvent(11 * IN_MILLISECONDS, [this]() -> void
        {
            me->RemoveAura(Withering3Percent);
            me->CastSpell(me, Withering5Percent, true);
        });

        AddDelayedEvent(16 * IN_MILLISECONDS, [this]() -> void
        {
            me->CastSpell(me, Withering3Percent, true);
            me->CastSpell(me, Withering5Percent, true);
        });

        AddDelayedEvent(30 * IN_MILLISECONDS, [this]() -> void { me->DespawnOrUnsummon(); });
    }

    void HealReceived(Unit* /*healer*/, uint32& heal) override
    {
        if ((me->GetHealth() + heal) >= me->GetMaxHealth() && !me->HasAura(LivingSpores))
            me->CastSpell(me, LivingSpores, true);
    }

    void DamageTaken(Unit* attacker, uint32& damage, DamageEffectType dmgType) override
    {
        if (attacker != me)
            damage = 0;
    }

    void JustDied(Unit* /*killer*/) override
    {
        m_Instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

        me->DespawnOrUnsummon(20 * IN_MILLISECONDS);
    }
};

/// Rejuvenating Mushroom - 78868
struct npc_highmaul_rejuvenating_mushroom : public MS::AI::CosmeticAI
{
    enum eSpells
    {
        RejuvenatingMushroomVisual = 159253,
        RejuvenatingMushroomL1Visual = 164246,
        Withering3Percent = 163122,
        Withering5Percent = 163124,
        RejuvenatingSpores = 159292
    };

    npc_highmaul_rejuvenating_mushroom(Creature* creature) : MS::AI::CosmeticAI(creature)
    {
        m_Instance = creature->GetInstanceScript();
    }

    InstanceScript* m_Instance;

    void Reset() override
    {
        me->setRegeneratingHealth(false);
        me->SetHealth(me->GetMaxHealth() / 2);
        me->AddUnitState(UNIT_STATE_STUNNED);

        me->CastSpell(me, RejuvenatingMushroomVisual, true);
        me->CastSpell(me, RejuvenatingMushroomL1Visual, true);

        m_Instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me, 4);

        AddDelayedEvent(6 * IN_MILLISECONDS, [this]() -> void { me->CastSpell(me, Withering3Percent, true); });

        AddDelayedEvent(11 * IN_MILLISECONDS, [this]() -> void
        {
            me->RemoveAura(Withering3Percent);
            me->CastSpell(me, Withering5Percent, true);
        });

        AddDelayedEvent(16 * IN_MILLISECONDS, [this]() -> void
        {
            me->CastSpell(me, Withering3Percent, true);
            me->CastSpell(me, Withering5Percent, true);
        });

        AddDelayedEvent(30 * IN_MILLISECONDS, [this]() -> void { me->DespawnOrUnsummon(); });
    }

    void HealReceived(Unit* /*healer*/, uint32& heal) override
    {
        if ((me->GetHealth() + heal) >= me->GetMaxHealth() && !me->HasAura(RejuvenatingSpores))
            me->CastSpell(me, RejuvenatingSpores, true);
    }

    void DamageTaken(Unit* attacker, uint32& damage, DamageEffectType dmgType) override
    {
        if (attacker != me)
            damage = 0;
    }

    void JustDied(Unit* /*killer*/) override
    {
        m_Instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

        me->DespawnOrUnsummon(20 * IN_MILLISECONDS);
    }
};

/// BFC9000 - 81403
struct npc_highmaul_bfc9000 : public ScriptedAI
{
    enum eSpells
    {
        Flamethrower = 163663,
        BFC9000 = 164175
    };

    npc_highmaul_bfc9000(Creature* creature) : ScriptedAI(creature) { }

    void Reset() override
    {
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_IMMUNE_TO_NPC);
        me->SetFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);

        me->SetReactState(REACT_PASSIVE);
    }

    void OnSpellClick(Unit* clicker) override
    {
        if (!me->HasAura(BFC9000) || clicker->HasAura(Flamethrower))
            return;

        clicker->CastSpell(clicker, Flamethrower, true, nullptr, nullptr, me->GetGUID());
        me->RemoveAura(BFC9000);
    }
};

/// Necrotic Breath - 159220
class spell_highmaul_necrotic_breath : public SpellScript
{
    PrepareSpellScript(spell_highmaul_necrotic_breath);

    enum eSpell
    {
        TargetRestrict = 20036
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
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_highmaul_necrotic_breath::CorrectTargets, EFFECT_0, TARGET_UNIT_CONE_ENEMY_24);
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_highmaul_necrotic_breath::CorrectTargets, EFFECT_1, TARGET_UNIT_CONE_ENEMY_24);
    }
};

/// Flamethrower (aura) - 163322
class spell_highmaul_flamethrower_aura_AuraScript : public AuraScript
{
    enum eSpells
    {
        FlamethrowerRegen = 163667,
        PulsingHeat = 163666
    };

    PrepareAuraScript(spell_highmaul_flamethrower_aura_AuraScript);

    void OnTick(AuraEffect const* auraEffect)
    {
        if (Unit* target = GetTarget())
        {
            if (target->GetPower(POWER_ALTERNATE) >= 100)
            {
                target->CastSpell(target, PulsingHeat, true);
                auraEffect->GetBase()->Remove(AURA_REMOVE_BY_CANCEL);
            }
        }
    }

    void OnRemove(AuraEffect const* /*auraEffect*/, AuraEffectHandleModes /*mode*/)
    {
        AuraRemoveMode removeMode = GetTargetApplication()->GetRemoveMode();
        if (removeMode == AURA_REMOVE_BY_CANCEL)
            return;

        if (Unit* target = GetTarget())
            target->CastSpell(target, FlamethrowerRegen, true);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_highmaul_flamethrower_aura_AuraScript::OnTick, EFFECT_2, SPELL_AURA_OBS_MOD_POWER);
        AfterEffectRemove += AuraEffectRemoveFn(spell_highmaul_flamethrower_aura_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_MOD_PACIFY_SILENCE, AURA_EFFECT_HANDLE_REAL);
    }
};


class spell_highmaul_flamethrower_aura : public SpellScript
{
    enum eSpells
    {
        FlamethrowerRegen = 163667,
        PulsingHeat = 163666
    };

    PrepareSpellScript(spell_highmaul_flamethrower_aura);

    SpellCastResult CheckPulsingHeat()
    {
        if (Unit* caster = GetCaster())
        {
            if (caster->HasAura(PulsingHeat))
            {
                SetCustomCastResultMessage(SPELL_CUSTOM_ERROR_YOUR_WEAPON_HAS_OVERHEATED);
                return SPELL_FAILED_CUSTOM_ERROR;
            }
            else if (caster->HasAura(GetSpellInfo()->Id))
            {
                caster->RemoveAura(GetSpellInfo()->Id);
                return SPELL_FAILED_DONT_REPORT;
            }
        }

        return SPELL_CAST_OK;
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_highmaul_flamethrower_aura::CheckPulsingHeat);
    }
};

/// Flamethrower "Regen" - 163667
class spell_highmaul_flamethrower_regen : public AuraScript
{
    PrepareAuraScript(spell_highmaul_flamethrower_regen);

    void OnTick(AuraEffect const* /*auraEffect*/)
    {
        if (Unit* target = GetTarget())
            target->ModifyPower(POWER_ALTERNATE, -2);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_highmaul_flamethrower_regen::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }
};

/// Pulsing Heat - 163666
class spell_highmaul_pulsing_heat : public AuraScript
{
    PrepareAuraScript(spell_highmaul_pulsing_heat);

    enum eSpell
    {
        FlamethrowerRegen = 163667
    };

    void OnRemove(AuraEffect const* /*auraEffect*/, AuraEffectHandleModes /*mode*/)
    {
        if (Unit* target = GetTarget())
            target->CastSpell(target, FlamethrowerRegen, true);
    }

    void Register() override
    {
        AfterEffectRemove += AuraEffectRemoveFn(spell_highmaul_pulsing_heat::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

/// Creeping Moss 1 - 163347
class spell_highmaul_creeping_moss : public AuraScript
{
    PrepareAuraScript(spell_highmaul_creeping_moss);

    enum eAction
    {
        CreepingMoss = 1
    };

    void OnTick(AuraEffect const* /*auraEffect*/)
    {
        if (GetTarget() == nullptr)
            return;

        if (Creature* target = GetTarget()->ToCreature())
        {
            if (target->GetAI())
                target->AI()->DoAction(CreepingMoss);
        }
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_highmaul_creeping_moss::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }
};

/// Flamethrower - 163310
class spell_highmaul_flamethrower : public SpellScript
{
    enum eSpells
    {
        CreepingMoss = 173229,
        BurningInfusion = 165223,
        Flamethrower = 173281
    };

    PrepareSpellScript(spell_highmaul_flamethrower);

    void HandleDummy(SpellEffIndex /*effectIndex*/)
    {
    }

    void Register() override
    {
        OnEffectHit += SpellEffectFn(spell_highmaul_flamethrower::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

/// Burning Infusion - 165223
class spell_highmaul_burning_infusion : public AuraScript
{
    PrepareAuraScript(spell_highmaul_burning_infusion);

    void OnTick(AuraEffect const* auraEffect)
    {
        auraEffect->GetBase()->ModStackAmount(-1, AURA_REMOVE_BY_DEFAULT);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_highmaul_burning_infusion::OnTick, EFFECT_3, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
    }
};

/// Energy Regen - 164248
class spell_highmaul_energy_regen : public AuraScript
{
    enum eAction
    {
        InfestingSpores = 2
    };

    PrepareAuraScript(spell_highmaul_energy_regen);

    void OnTick(AuraEffect const* /*auraEffect*/)
    {
        if (Unit* target = GetTarget())
        {
            target->EnergizeBySpell(target, GetSpellInfo()->Id, 10, POWER_RAGE);

            if (Creature* boss = target->ToCreature())
                if (boss->IsAIEnabled && boss->GetPowerPct(POWER_RAGE) >= 100.0f)
                    boss->AI()->DoAction(InfestingSpores);
        }
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_highmaul_energy_regen::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }
};

/// Spore Shot - 173244
class spell_highmaul_spore_shot : public SpellScript
{
    PrepareSpellScript(spell_highmaul_spore_shot);

    void CorrectTargets(std::list<WorldObject*>& targets)
    {
        if (targets.empty())
            return;

        targets.remove_if([this](WorldObject* object) -> bool
        {
            if (object == nullptr)
                return true;

            /// Should always hit a player if possible
            if (object->IsPlayer())
                return false;
            else if (Creature* creature = object->ToCreature())
                if (creature->GetOwner() == nullptr)
                    return true;

            return false;
        });
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_highmaul_spore_shot::CorrectTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENTRY);
    }
};

/// Flamethrower (overrider) - 163663
class spell_highmaul_flamethrower_overrider : public AuraScript
{
    enum eSpell
    {
        BFC9000 = 164175
    };

    PrepareAuraScript(spell_highmaul_flamethrower_overrider);

    void OnRemove(AuraEffect const* /*auraEffect*/, AuraEffectHandleModes /*mode*/)
    {
        AuraRemoveMode removeMode = GetTargetApplication()->GetRemoveMode();
        if (removeMode != AURA_REMOVE_BY_DEATH || GetCaster() == nullptr)
            return;

        if (Creature* l_BFC9000 = GetCaster()->ToCreature())
        {
            l_BFC9000->Respawn(true);
            l_BFC9000->CastSpell(l_BFC9000, BFC9000, true);

            if (l_BFC9000->IsAIEnabled)
                l_BFC9000->AI()->Reset();
        }
    }

    void Register() override
    {
        AfterEffectRemove += AuraEffectRemoveFn(spell_highmaul_flamethrower_overrider::OnRemove, EFFECT_0, SPELL_AURA_ENABLE_ALT_POWER, AURA_EFFECT_HANDLE_REAL);
    }
};

void AddSC_boss_brackenspore()
{
    RegisterHighmaulCreatureAI(boss_brackenspore);
    RegisterHighmaulCreatureAI(npc_highmaul_mind_fungus);
    RegisterHighmaulCreatureAI(npc_highmaul_spore_shooter);
    RegisterHighmaulCreatureAI(npc_highmaul_fungal_flesh_eater);
    RegisterHighmaulCreatureAI(npc_highmaul_living_mushroom);
    RegisterHighmaulCreatureAI(npc_highmaul_rejuvenating_mushroom);
    RegisterHighmaulCreatureAI(npc_highmaul_bfc9000);

    RegisterSpellScript(spell_highmaul_necrotic_breath);
    RegisterSpellAndAuraScriptPair(spell_highmaul_flamethrower_aura, spell_highmaul_flamethrower_aura_AuraScript);
    RegisterAuraScript(spell_highmaul_flamethrower_regen);
    RegisterAuraScript(spell_highmaul_pulsing_heat);
    RegisterAuraScript(spell_highmaul_creeping_moss);
    RegisterSpellScript(spell_highmaul_flamethrower);
    RegisterAuraScript(spell_highmaul_burning_infusion);
    RegisterAuraScript(spell_highmaul_energy_regen);
    RegisterSpellScript(spell_highmaul_spore_shot);
    RegisterAuraScript(spell_highmaul_flamethrower_overrider);
}
