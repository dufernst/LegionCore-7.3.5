////////////////////////////////////////////////////////////////////////////////
//
//  MILLENIUM-STUDIO
//  Copyright 2016 Millenium-studio SARL
//  All Rights Reserved.
//
////////////////////////////////////////////////////////////////////////////////

# include "highmaul.hpp"

Position const g_MaggotSpawnPos[MaxMaggotToKill] =
{
    {3827.051f, 7690.205f, 23.67708f, 0.000f},
    {3751.507f, 7722.538f, 23.65485f, 4.332f},
    {3704.671f, 7700.851f, 23.60431f, 3.872f},
    {3687.367f, 7626.960f, 23.64627f, 1.488f},
    {3743.118f, 7607.410f, 23.86531f, 5.192f},
    {3799.805f, 7675.845f, 23.04378f, 3.110f}
};

G3D::Vector3 ComputeLocationSelection(Creature* source, float searchRange, float minRadius, float radius)
{
    using Cluster = std::set<Player*>;
    using GuidCluster = std::set<uint32>;

    std::list<Player*> targets;

    source->GetPlayerListInGrid(targets, searchRange);

    if (!targets.empty())
    {
        targets.remove_if([source, minRadius](Player* player) -> bool
        {
            if (player == nullptr)
                return true;

            if (player->GetDistance(source) < minRadius)
                return true;

            return false;
        });
    }

    std::map<uint32, Cluster> clusterMap;

    for (Player* player : targets)
    {
        Cluster l_Neighboor;
        for (Player* l_PlayerSecond : targets)
        {
            float dist = player->GetDistance(l_PlayerSecond);
            if (dist <= radius)
                l_Neighboor.insert(l_PlayerSecond);
        }

        clusterMap[player->GetGUIDLow()] = Cluster();
        clusterMap[player->GetGUIDLow()].insert(player);

        for (Player* l_PlayerSecond : l_Neighboor)
            clusterMap[player->GetGUIDLow()].insert(l_PlayerSecond);
    }

    size_t size = 0;
    for (auto l_Cluster : clusterMap)
    {
        if (l_Cluster.second.size() > size)
            size = l_Cluster.second.size();
    }

    G3D::Vector3 pos;
    for (auto l_Cluster : clusterMap)
    {
        if (size && l_Cluster.second.size() == size)
        {
            float x = 0.0f;
            float y = 0.0f;

            for (Player* player : l_Cluster.second)
            {
                x += player->GetPositionX();
                y += player->GetPositionY();
            }

            x /= (uint32)size;
            y /= (uint32)size;

            pos.x = x;
            pos.y = y;
            pos.z = source->GetPositionZ();
            break;
        }
    }

    return pos;
}

G3D::Vector3 GetCleaveLocation(Creature* source)
{
    return ComputeLocationSelection(source, 5.0f, 0.0f, 5.0f);
}

G3D::Vector3 GetBoundingCleaveLocation(Creature* source)
{
    return ComputeLocationSelection(source, 500.0f, 5.1f, 10.0f);
}

/// The Butcher - 77404
struct boss_the_butcher : public BossAI
{
    enum eSpells
    {
        /// Heavy Handed
        HeavyHandedAura = 156135,
        HeavyHandedProc = 156138,
        /// The Cleaver
        TheCleaverDmg = 156143,
        TheCleaverDot = 156147,
        /// The Tenderizer
        TheTenderizer = 156151,
        /// Meat Hook
        MeatHook = 156125,
        MeatHookJump = 156127,
        /// Bounding Cleave
        BoundingCleaveKnock = 156160,
        BoundingCleaveDummy = 156197,
        BoundingCleaveDmg = 156172,
        BoundingCleaveCharg = 156171,
        /// Cleave
        SpellCleave = 156157,
        SpellGushingWounds = 156152,
        /// Energy management
        Angry5PerTick = 156720,
        Angry10PerTick = 160248,
        /// Frenzy (30%)
        SpellFrenzy = 156598,
        /// Loot
        ButcherBonusLoot = 177522
    };

    enum eEvents
    {
        EventTenderizer = 1,
        EventCleave,
        EventCleaver,
        EventBerserk,
        EventBoundingCleave,
        EventMeatHook,
        /// Mythic mode only
        EventCadaver
    };

    enum eTalks
    {
        Intro1,
        Intro2,
        Intro3, ///< Didn't see it on video
        Aggro,
        BoundingCleave,
        Frenzy,
        Cleave,
        Berserk,
        Slay,
        Wipe
    };

    enum eCreatures
    {
        NightTwistedCadaver = 82505,
        Maggot = 80728
    };

    enum eAction
    {
        MaggotKilled
    };

    boss_the_butcher(Creature* creature) : BossAI(creature, BossTheButcher)
    {
        m_Instance = creature->GetInstanceScript();
        m_IntroDone = false;
    }

    EventMap m_Events;
    InstanceScript* m_Instance;
    bool m_IntroDone;
    uint32 m_CleaveCooldown;
    uint8 m_AddCount;
    std::set<uint8> m_MaggotSpawned;

    void Reset() override
    {
        m_Events.Reset();

        _Reset();

        me->RemoveAura(SpellFrenzy);
        me->RemoveAura(Berserker);
        me->RemoveAura(Angry5PerTick);
        me->RemoveAura(Angry10PerTick);

        me->SetSpeed(MOVE_SWIM, me->GetSpeed(MOVE_RUN) * 0.5f);

        m_CleaveCooldown = 0;

        m_AddCount = 0;

        me->SetCanDualWield(true);

        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISARMED | UNIT_FLAG_REMOVE_CLIENT_CONTROL | UNIT_FLAG_PREPARATION);

        me->SetPower(POWER_ENERGY, 0);
        me->SetMaxPower(POWER_ENERGY, 100);
        me->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_REGENERATE_POWER);

        m_Instance->DoRemoveAurasDueToSpellOnPlayers(TheCleaverDot);
        m_Instance->DoRemoveAurasDueToSpellOnPlayers(TheTenderizer);
        m_Instance->DoRemoveAurasDueToSpellOnPlayers(SpellGushingWounds);

        m_MaggotSpawned.clear();
    }

    void EnterCombat(Unit* /*attacker*/) override
    {
        _EnterCombat();

        me->CastSpell(me, Angry5PerTick, true);

        Talk(Aggro);

        m_Events.RescheduleEvent(EventTenderizer, 6 * IN_MILLISECONDS);
        m_Events.RescheduleEvent(EventCleave, 10 * IN_MILLISECONDS);
        m_Events.RescheduleEvent(EventCleaver, 12 * IN_MILLISECONDS);

        if (!m_Instance->instance->IsLfr())
            m_Events.RescheduleEvent(EventBerserk, IsMythicRaid() ? 240 * IN_MILLISECONDS : 300 * IN_MILLISECONDS);

        /// Meat Hook is an ability that The Butcher uses to pull his tank to him.
        /// We assume that this ability exists to prevent The Butcher from being kited,
        /// but it is not otherwise in use during the fight.
        m_Events.RescheduleEvent(EventMeatHook, 5 * IN_MILLISECONDS);

        /// Mythic mode only
        if (IsMythicRaid())
        {
            m_AddCount = 0;
            m_Events.RescheduleEvent(EventCadaver, 18 * IN_MILLISECONDS);
        }

        if (!m_Instance->instance->IsLfr())
        {
            uint8 i = urand(0, (MaxMaggotToKill - 1));

            me->SummonCreature(Maggot, g_MaggotSpawnPos[i]);
            m_MaggotSpawned.insert(i);
        }
    }

    bool CanRespawn() override
    {
        return false;
    }

    void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType dmgType) override
    {
        if (me->HasAura(SpellFrenzy))
            return;

        if (me->HealthBelowPctDamaged(30, damage))
        {
            me->CastSpell(me, SpellFrenzy, true);

            me->RemoveAura(Angry5PerTick);
            me->CastSpell(me, Angry10PerTick, true);

            Talk(Frenzy);
        }
    }

    void EnterEvadeMode() override
    {
        CreatureAI::EnterEvadeMode();

        Talk(Wipe);

        m_Instance->SetBossState(BossTheButcher, FAIL);
    }

    void RegeneratePower(Powers /*power*/, float& value) override
    {
        /// The Butcher only regens by script
        value = 0;
    }

    void JustDied(Unit* /*killer*/) override
    {
        _JustDied();

        m_Instance->DoRemoveAurasDueToSpellOnPlayers(TheCleaverDot);
        m_Instance->DoRemoveAurasDueToSpellOnPlayers(TheTenderizer);
        m_Instance->DoRemoveAurasDueToSpellOnPlayers(SpellGushingWounds);

        CastSpellToPlayers(me->GetMap(), me, ButcherBonusLoot, true);
    }

    void KilledUnit(Unit* killed) override
    {
        if (killed->IsPlayer())
            Talk(Slay);
    }

    void MovementInform(uint32 type, uint32 id) override
    {
        if (type != MovementGeneratorType::EFFECT_MOTION_TYPE)
            return;

        if (id == BoundingCleaveCharg)
        {
            me->CastSpell(me, BoundingCleaveDmg, true);

            me->RemoveAura(BoundingCleaveDummy);

            AddDelayedEvent(50, [this]() -> void
            {
                if (Unit* target = me->getVictim())
                {
                    me->GetMotionMaster()->Clear();
                    me->GetMotionMaster()->MoveChase(target);
                }
            });
        }
    }

    void SpellMissTarget(Unit* target, SpellInfo const* spellInfo, SpellMissInfo missInfo) override
    {
        if (target == nullptr)
            return;

        switch (spellInfo->Id)
        {
            case TheCleaverDmg:
            {
                if (missInfo == SPELL_MISS_DODGE)
                    break;

                me->AddAura(TheCleaverDot, target);
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
            case MeatHook:
                target->CastSpell(me, MeatHookJump, true);
                break;
            case SpellCleave:
            case BoundingCleaveDmg:
                me->CastSpell(target, SpellGushingWounds, true);
                break;
            default:
                break;
        }
    }

    void SpellFinishCast(SpellInfo const* spellInfo) override
    {
        if (spellInfo->Id == BoundingCleaveCharg)
        {
            AddDelayedEvent(2 * IN_MILLISECONDS, [this]() -> void
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                    AttackStart(target);

                if (Creature* maggot = me->FindNearestCreature(Maggot, 10.0f))
                    me->Kill(maggot);
            });
        }
    }

    void DoAction(int32 const action) override
    {
        if (action == MaggotKilled)
        {
            std::vector<uint8> indexes = {0, 1, 2, 3, 4, 5};
            std::random_shuffle(indexes.begin(), indexes.end());

            for (uint8 i : indexes)
            {
                if (m_MaggotSpawned.find(i) != m_MaggotSpawned.end())
                    continue;

                me->SummonCreature(Maggot, g_MaggotSpawnPos[i]);
                m_MaggotSpawned.insert(i);
                break;
            }

            m_Instance->SetData(ButcherAchievement, 1);
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (!m_IntroDone)
        {
            if (Player* player = me->FindNearestPlayer(130.0f))
            {
                m_IntroDone = true;
                AddDelayedEvent(1 * IN_MILLISECONDS, [this]() -> void { Talk(Intro1); });
                AddDelayedEvent(8 * IN_MILLISECONDS, [this]() -> void { Talk(Intro2); });
            }
        }

        if (!UpdateVictim())
            return;

        ScheduleEnergy(diff);

        m_Events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        switch (m_Events.ExecuteEvent())
        {
            case EventTenderizer:
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                    me->CastSpell(target, TheTenderizer, true);
                m_Events.RescheduleEvent(EventTenderizer, 16 * IN_MILLISECONDS);
                break;
            }
            case EventCleave:
            {
                if (m_CleaveCooldown <= diff)
                {
                    m_CleaveCooldown = 20 * IN_MILLISECONDS;
                    Talk(Cleave);
                }
                else
                    m_CleaveCooldown -= diff;

                me->CastSpell(GetCleaveLocation(me), SpellCleave, false);

                m_Events.RescheduleEvent(EventCleave, 5 * IN_MILLISECONDS);
                break;
            }
            case EventCleaver:
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                    me->CastSpell(target, TheCleaverDmg, true);
                m_Events.RescheduleEvent(EventCleaver, 8 * IN_MILLISECONDS);
                break;
            }
            case EventBerserk:
            {
                Talk(Berserk);
                me->CastSpell(me, Berserker, true);
                break;
            }
            case EventBoundingCleave:
            {
                Talk(BoundingCleave);
                me->CastSpell(me, BoundingCleaveKnock, true);
                me->CastSpell(me, BoundingCleaveDummy, false);
                m_Events.DelayEvents(12 * IN_MILLISECONDS);
                break;
            }
            case EventMeatHook:
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                    if (!target->IsWithinMeleeRange(me, 5.0f))
                        me->CastSpell(target, MeatHook, true);
                m_Events.RescheduleEvent(EventMeatHook, 5 * IN_MILLISECONDS);
                break;
            }
            case EventCadaver:
            {
                /// Every four waves of adds, The Butcher spawns one add more
                ++m_AddCount;
                uint8 count = uint8(floor(float(m_AddCount) / 4.0f) + 1);

                float radius = 50.0f;
                float posX = me->GetHomePosition().m_positionX;
                float posY = me->GetHomePosition().m_positionY;
                float posZ = me->GetHomePosition().m_positionZ;

                for (uint8 i = 0; i < count; ++i)
                {
                    float o = frand(0, 2 * M_PI);
                    float x = posX + (radius * cos(o));
                    float y = posY + (radius * sin(o));

                    me->SummonCreature(NightTwistedCadaver, x, y, posZ);
                }

                m_Events.RescheduleEvent(EventCadaver, 14 * IN_MILLISECONDS);
                break;
            }
            default:
                break;
        }

        EnterEvadeIfOutOfCombatArea(diff);
        DoMeleeAttackIfReady();
    }

    void ScheduleEnergy(uint32 const /*diff*/)
    {
        if (me->GetPower(POWER_ENERGY) >= 100)
            m_Events.RescheduleEvent(EventBoundingCleave, 50);
    }
};

/// Night-Twisted Cadaver - 82505
struct npc_highmaul_night_twisted_cadaver : public MS::AI::CosmeticAI
{
    enum eSpells
    {
        Paleobomb = 163047,
        PaleVitriol = 163042
    };

    npc_highmaul_night_twisted_cadaver(Creature* creature) : MS::AI::CosmeticAI(creature)
    {
        m_Instance = creature->GetInstanceScript();
    }

    InstanceScript* m_Instance;
    bool m_Explode;

    void Reset() override
    {
        m_Explode = false;

        me->SetReactState(REACT_PASSIVE);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);

        AddDelayedEvent(1 * IN_MILLISECONDS, [this]() -> void
        {
            if (!m_Instance)
                return;

            me->SetWalk(true);

            if (Creature* butcher = m_Instance->GetCreature((TheButcher)))
                me->GetMotionMaster()->MoveChase(butcher);
        });
    }

    void UpdateAI(uint32 diff) override
    {
        MS::AI::CosmeticAI::UpdateAI(diff);

        if (m_Explode)
            return;

        if (Creature* butcher = m_Instance->GetCreature((TheButcher)))
            if (me->GetDistance(butcher) <= 1.0f)
                ExplodeAndSpawnVitriol();

        if (Player* player = me->FindNearestPlayer(1.0f))
            ExplodeAndSpawnVitriol();
    }

    void ExplodeAndSpawnVitriol()
    {
        me->GetMotionMaster()->Clear();

        m_Explode = true;

        me->CastSpell(me, Paleobomb, true);
        me->CastSpell(me, PaleVitriol, true);

        AddDelayedEvent(1 * IN_MILLISECONDS, [this]() -> void { me->Kill(me); });
    }
};

/// Maggot - 80728
struct npc_highmaul_maggot : public ScriptedAI
{
    enum eAction
    {
        MaggotKilled
    };

    npc_highmaul_maggot(Creature* creature) : ScriptedAI(creature)
    {
        m_Instance = creature->GetInstanceScript();
    }

    InstanceScript* m_Instance;

    void Reset() override
    {
        me->SetReactState(REACT_PASSIVE);

        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_IMMUNE_TO_PC);
    }

    void JustDied(Unit* /*killer*/) override
    {
        if (Creature* butcher = m_Instance->GetCreature((TheButcher)))
            if (butcher->IsAIEnabled)
                butcher->AI()->DoAction(MaggotKilled);
    }
};


/// Heavy Handed - 156135
class spell_highmaul_heavy_handed : public AuraScript
{
    enum eSpell
    {
        HeavyHandedProc = 156138
    };

    PrepareAuraScript(spell_highmaul_heavy_handed);

    void OnProc(AuraEffect const* /*auraEffect*/, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();

        if (eventInfo.GetDamageInfo()->GetSpellInfo() != nullptr && eventInfo.GetDamageInfo()->GetSpellInfo()->Id == HeavyHandedProc)
            return;

        if (Unit* butcher = GetTarget())
            if (Unit* target = eventInfo.GetActionTarget())
                butcher->CastSpell(target, HeavyHandedProc, true);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_highmaul_heavy_handed::OnProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

/// Heavy Handed - 156138
class spell_highmaul_heavy_handed_proc : public SpellScript
{
    PrepareSpellScript(spell_highmaul_heavy_handed_proc);

    ObjectGuid m_Target;

    bool Load() override
    {
        return true;
    }

    void HandleBeforeCast()
    {
        if (Unit* target = GetExplTargetUnit())
            m_Target = target->GetGUID();
    }

    void CorrectTargets(std::list<WorldObject*>& targets)
    {
        if (targets.empty())
            return;

        Unit* caster = GetCaster();
        if (caster == nullptr)
            return;

        /// All attacks made by the caster strike the next closest target
        if (targets.size() > 1)
        {
            targets.remove_if([this](WorldObject* object) -> bool
            {
                if (object == nullptr || object->GetGUID() == m_Target)
                    return true;

                return false;
            });

            if (targets.size() > 1)
            {
                targets.sort(Trinity::ObjectDistanceOrderPred(caster));
                WorldObject* l_Object = targets.front();
                targets.clear();
                targets.push_back(l_Object);
            }
        }
        else
        {
            targets.clear();

            if (Unit* target = Unit::GetUnit(*caster, m_Target))
                targets.push_back(target);
        }
    }

    void Register() override
    {
        BeforeCast += SpellCastFn(spell_highmaul_heavy_handed_proc::HandleBeforeCast);
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_highmaul_heavy_handed_proc::CorrectTargets, EFFECT_1, TARGET_UNIT_DEST_AREA_ENEMY);
    }
};

/// Bounding Cleave (Periodic Dummy) - 156197
class spell_highmaul_bounding_cleave_dummy : public AuraScript
{
    enum eSpell
    {
        BoundingCleaveCharge = 156171
    };

    PrepareAuraScript(spell_highmaul_bounding_cleave_dummy);

    void OnTick(AuraEffect const* auraEffect)
    {
        if (auraEffect->GetTickNumber() != 8)
            return;

        if (GetTarget() == nullptr)
            return;

        Creature* butcher = GetTarget()->ToCreature();
        if (butcher == nullptr)
            return;

        G3D::Vector3 pos = GetBoundingCleaveLocation(butcher);
        if (pos.x == 0.0f || pos.y == 0.0f || pos.z == 0.0f)
            pos = ComputeLocationSelection(butcher, 500.0f, 0.0f, 10.0f);

        butcher->CastSpell(pos, BoundingCleaveCharge, true);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_highmaul_bounding_cleave_dummy::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }
};

/// Gushing Wounds - 156152
class spell_highmaul_gushing_wounds : public AuraScript
{
    PrepareAuraScript(spell_highmaul_gushing_wounds);

    enum eSpell
    {
        GushingWoundsKill = 156153
    };

    void AfterApply(AuraEffect const* auraEffect, AuraEffectHandleModes /*mode*/)
    {
        if (Unit* target = GetTarget())
        {
            Map* map = target->GetMap();
            Aura* aura = auraEffect->GetBase();

            if (map->IsLfr())
            {
                aura->SetDuration(10 * IN_MILLISECONDS);
                aura->SetMaxDuration(10 * IN_MILLISECONDS);
            }
            else
            {
                aura->SetDuration(15 * IN_MILLISECONDS);
                aura->SetMaxDuration(15 * IN_MILLISECONDS);
            }

            if (aura->GetStackAmount() >= 5)
                target->CastSpell(target, GushingWoundsKill, true);
        }
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(spell_highmaul_gushing_wounds::AfterApply, EFFECT_1, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
    }
};

void AddSC_boss_the_butcher()
{
    RegisterHighmaulCreatureAI(boss_the_butcher);
    RegisterHighmaulCreatureAI(npc_highmaul_night_twisted_cadaver);
    RegisterHighmaulCreatureAI(npc_highmaul_maggot);

    RegisterAuraScript(spell_highmaul_heavy_handed);
    RegisterSpellScript(spell_highmaul_heavy_handed_proc);
    RegisterAuraScript(spell_highmaul_bounding_cleave_dummy);
    RegisterAuraScript(spell_highmaul_gushing_wounds);
}
