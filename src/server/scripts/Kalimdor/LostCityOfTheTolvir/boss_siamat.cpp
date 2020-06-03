#include "lost_city_of_the_tolvir.h"
#include "AchievementMgr.h"

enum eSpells
{
    SPELL_ABSORB_STORMS              = 83151,
    SPELL_CALL_OF_SKY                = 84956,
    SPELL_DEFLECTING_WINDS           = 84589,
    SPELL_STORM_BOLT_PHASE_DW        = 73564,
    SPELL_STORM_BOLT_PHASE_S         = 91853,
    // Cloud Burst
    SPELL_CLOUD_BURST_SUMMON         = 83790,
    SPELL_CLOUD_BURST                = 83051,
    // Static Shock
    SPELL_STATIC_SHOCK_1             = 84546,
    SPELL_STATIC_SHOCK_2             = 84555,
    SPELL_STATIC_SHOCK_3             = 84556,
    // Wailing Winds
    SPELL_WAILING_WINDS              = 83094,
    SPELL_WAILING_WINDS_AURA         = 83066,
    // Servant of Siamat
    SPELL_THUNDER_CRASH              = 84521,
    SPELL_LIGHTNING_NOVA             = 84544,
    // Siamat Minion
    SPELL_CHAIN_LIGHTNING            = 83455,
    SPELL_TEMPEST_STORM_SUMMON       = 83414,
    SPELL_TEMPEST_STORM_TRANSFORM    = 83170,
    SPELL_LIGHTNING_CHARGE           = 91872,
    SPELL_LIGHTNING_CHARGE_AURA      = 93959,
    
    SPELL_ACHIEV_CREDIT              = 93957
};

enum eCreatures
{
    NPC_TEMPEST_STORM                = 44713,
    NPC_SETVANT_OF_SIAMAT            = 45269
};

enum eActions
{
    ACTION_SERVANT_DEATH
};

enum eTexts
{
    SAY_START                          = 0,
    SAY_AGGRO                          = 1,
    SAY_WAILING_WINDS                  = 2,
    SAY_KILL_PLAYER                    = 3,
    SAY_DEATH                          = 4
};

enum ePhases
{
    PHASE_DEFLECTING_WINDS           = 1,
    PHASE_WAILING_WINDS              = 2,
    PHASE_SIAMAT                     = 3,

    PHASE_WAILING_WINDS_MASK         = 1 << PHASE_WAILING_WINDS
};

enum eEvents
{
    // Siamat
    EVENT_STATIC_SHOCK               = 1,
    EVENT_DEFLECTING_WINDS           = 2,
    EVENT_CLOUD_BURST                = 3,
    EVENT_CALL_OF_SKY                = 4,
    EVENT_WAILING_WINDS              = 5,
    EVENT_STORM_BOLT_DW              = 6,
    EVENT_STORM_BOLT_S               = 7,
    EVENT_ABSORB_STORMS              = 8,
    // Servant of Siamat
    EVENT_THUNDER_CRASH              = 1,
    EVENT_LIGHTNING_NOVA             = 2,
    EVENT_SERVANT_DIED               = 3,
    // Siamat Minion
    EVENT_CHAIN_LIGHTNING            = 1,
    // Cloud Burst
    EVENT_PERIODIC_CAST              = 1
};

const uint32 StaticShock[3]=
{
    SPELL_STATIC_SHOCK_1,
    SPELL_STATIC_SHOCK_2,
    SPELL_STATIC_SHOCK_3,
};

#define    FLOR_COORD_Z    36.0f

struct boss_siamat : public ScriptedAI
{
    explicit boss_siamat(Creature* creature) : ScriptedAI(creature), summons(me)
    {
        instance = creature->GetInstanceScript();
        me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
        me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
    }

    EventMap events;
    SummonList summons;
    InstanceScript* instance;
    uint8 uiStaticShockId;

    void Reset() override
    {
        if (instance)
        {
            instance->SetData(DATA_SIAMAT, NOT_STARTED);
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_ACHIEV_CREDIT);
        }

        events.Reset();
        summons.DespawnAll();
        uiStaticShockId = 0;
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);
    }

    void EnterCombat(Unit* /*who*/) override
    {
        Talk(SAY_AGGRO);
        events.SetPhase(PHASE_DEFLECTING_WINDS);
        events.ScheduleEvent(EVENT_STATIC_SHOCK, 2000, 0, PHASE_DEFLECTING_WINDS);
        events.ScheduleEvent(EVENT_DEFLECTING_WINDS, 5000, 0, PHASE_DEFLECTING_WINDS);
        events.ScheduleEvent(EVENT_CALL_OF_SKY, 15000);
        events.ScheduleEvent(EVENT_CLOUD_BURST, 7000, 0, PHASE_DEFLECTING_WINDS);
        events.ScheduleEvent(EVENT_STORM_BOLT_DW, 500, 0, PHASE_DEFLECTING_WINDS);

        if (instance)
        {
            instance->SetData(DATA_SIAMAT, IN_PROGRESS);
            instance->DoCastSpellOnPlayers(SPELL_ACHIEV_CREDIT);
        }
    }

    void DoAction(int32 const action) override
    {
        if (action == ACTION_SERVANT_DEATH)
        {
            Talk(SAY_WAILING_WINDS);
            me->RemoveAura(SPELL_DEFLECTING_WINDS);
            DoCast(SPELL_WAILING_WINDS_AURA);
            events.SetPhase(PHASE_WAILING_WINDS);
            events.ScheduleEvent(EVENT_WAILING_WINDS, 1000, 0, PHASE_WAILING_WINDS);
            events.ScheduleEvent(EVENT_STORM_BOLT_S, urand(10000, 25000), 0, PHASE_SIAMAT);
        }
    }

    void JustSummoned(Creature* summoned) override
    {
        summons.Summon(summoned);
    }

    void KilledUnit(Unit* victim) override
    {
        if (!victim)
            return;

        if (victim->IsPlayer())
            Talk(SAY_KILL_PLAYER);
    }

    void JustDied(Unit* /*killer*/) override
    {
        events.Reset();
        summons.DespawnAll();
        Talk(SAY_DEATH);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING) && !events.IsInPhase(PHASE_WAILING_WINDS))
            return;

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_STATIC_SHOCK:
            {
                uint8 dist = urand(5, 30);
                float angle = frand(0, M_PI);
                float x, y;
                me->GetNearPoint2D(x, y, (float)dist, angle);
                me->CastSpell(x, y, FLOR_COORD_Z, StaticShock[uiStaticShockId], false);
                ++uiStaticShockId;

                if (uiStaticShockId <= 3)
                    events.ScheduleEvent(EVENT_STATIC_SHOCK, 32000, 0, PHASE_DEFLECTING_WINDS);
                break;
            }
            case EVENT_DEFLECTING_WINDS:
                DoCast(SPELL_DEFLECTING_WINDS);
                break;
            case EVENT_CALL_OF_SKY:
            {
                uint8 dist = urand(5, 30);
                float angle = frand(0, M_PI);
                float x, y;
                me->GetNearPoint2D(x, y, (float)dist, angle);
                me->CastSpell(x, y, FLOR_COORD_Z, SPELL_CALL_OF_SKY, false);
                events.ScheduleEvent(EVENT_CALL_OF_SKY, urand(15000, 35000));
                break;
            }
            case EVENT_CLOUD_BURST:
                if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                {
                    float x, y, z;
                    target->GetPosition(x, y, z);
                    me->CastSpell(x, y, z, SPELL_CLOUD_BURST_SUMMON, false);
                }
                events.ScheduleEvent(EVENT_CLOUD_BURST, urand(10000, 25000), 0, PHASE_DEFLECTING_WINDS);
                break;
            case EVENT_WAILING_WINDS:
            {
                DoCast(SPELL_WAILING_WINDS);

                if (Spell* spell = me->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
                    if (SpellInfo const* spellInfo = spell->GetSpellInfo())
                        if (spellInfo->Id == SPELL_WAILING_WINDS_AURA)
                            events.ScheduleEvent(EVENT_WAILING_WINDS, 1000, 0, PHASE_WAILING_WINDS);
                else
                {
                    events.SetPhase(PHASE_SIAMAT);
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);

                    if (auto victim = me->getVictim())
                        me->GetMotionMaster()->MoveChase(victim);

                    events.ScheduleEvent(EVENT_ABSORB_STORMS, 15000, 0, PHASE_SIAMAT);
                    events.ScheduleEvent(EVENT_STORM_BOLT_S, urand(10000, 25000), 0, PHASE_SIAMAT);
                }
            }
            break;
            case EVENT_ABSORB_STORMS:
                DoCast(SPELL_ABSORB_STORMS);
                events.ScheduleEvent(EVENT_ABSORB_STORMS, 33000, 0, PHASE_SIAMAT);
                break;
            case EVENT_STORM_BOLT_DW:
                if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                    DoCast(target, SPELL_STORM_BOLT_PHASE_DW, false);

                events.ScheduleEvent(EVENT_STORM_BOLT_DW, 2500, 0, PHASE_DEFLECTING_WINDS);
                break;
            case EVENT_STORM_BOLT_S:
                if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                    DoCast(target, SPELL_STORM_BOLT_PHASE_S, false);

                events.ScheduleEvent(EVENT_STORM_BOLT_S, urand(10000, 25000), 0, PHASE_SIAMAT);
                break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

struct npc_servant_of_siamat : public ScriptedAI
{
    explicit npc_servant_of_siamat(Creature* creature) : ScriptedAI(creature)
    {
        instance = creature->GetInstanceScript();
        me->SetInCombatWithZone();
    }

    EventMap events;
    InstanceScript* instance;
    bool LightningCharge;

    void Reset() override
    {
        events.Reset();
        LightningCharge = false;
    }

    void EnterCombat(Unit* /*who*/) override
    {
        instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
        events.ScheduleEvent(EVENT_THUNDER_CRASH, 1000);
        events.ScheduleEvent(EVENT_LIGHTNING_NOVA, 5000);
    }

    void DamageTaken(Unit* /*attacker*/, uint32 &damage, DamageEffectType /*dmgType*/) override
    {
        if (!IsHeroic())
            return;

        if (damage >= me->GetHealth())
        {
            damage = 0;
            me->SetHealth(1);

            if (!LightningCharge)
            {
                if (instance)
                    me->CastSpell(me, SPELL_LIGHTNING_CHARGE, false, 0, 0, instance->GetGuidData(DATA_SIAMAT));

                LightningCharge = true;
                me->SetReactState(REACT_PASSIVE);
                me->AttackStop();
                events.Reset();
                events.ScheduleEvent(EVENT_SERVANT_DIED, 2000);
            }
        }
    }

    void JustDied(Unit* /*killer*/) override
    {
        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

        if (me->GetEntry() == NPC_SETVANT_OF_SIAMAT)
            if (auto siamat = me->FindNearestCreature(BOSS_SIAMAT, 300.0f))
                siamat->AI()->DoAction(ACTION_SERVANT_DEATH);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_SERVANT_DIED:
                me->Kill(me);
                break;
            case EVENT_THUNDER_CRASH:
                DoCast(SPELL_THUNDER_CRASH);
                events.ScheduleEvent(EVENT_THUNDER_CRASH, urand(7000, 15000));
                break;
            case EVENT_LIGHTNING_NOVA:
                if (auto victim = me->getVictim())
                    DoCast(victim, SPELL_LIGHTNING_NOVA, false);

                events.ScheduleEvent(EVENT_LIGHTNING_NOVA, urand(7000, 15000));
                break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

struct npc_siamat_minion : public ScriptedAI
{
    explicit npc_siamat_minion(Creature* creature) : ScriptedAI(creature)
    {
        me->SetInCombatWithZone();
        instance = me->GetInstanceScript();
    }

    InstanceScript* instance;
    EventMap events;
    bool TempestStorm;

    void Reset() override
    {
        events.Reset();
        events.ScheduleEvent(EVENT_CHAIN_LIGHTNING, 1000);
        TempestStorm = false;
        me->AddAura(84550, me);
    }

    void JustSummoned(Creature* summoned) override
    {
        if (summoned->GetEntry() == NPC_TEMPEST_STORM)
        {
            summoned->SetReactState(REACT_PASSIVE);
            summoned->SetInCombatWithZone();
            summoned->GetMotionMaster()->MoveRandom(25.0f);

            if (instance)
                if (auto siamat = Unit::GetCreature(*me, instance->GetGuidData(DATA_SIAMAT)))
                    siamat->AI()->JustSummoned(summoned);
        }
    }

    void SpellHit(Unit* /*caster*/, const SpellInfo* spell) override
    {
        if (spell->Id == SPELL_TEMPEST_STORM_TRANSFORM)
        {
            DoCast(SPELL_TEMPEST_STORM_SUMMON);
            me->DespawnOrUnsummon(100);
        }
    }

    void DamageTaken(Unit* /*attacker*/, uint32 &damage, DamageEffectType /*dmgType*/) override
    {
        if (me->HealthBelowPctDamaged(2, damage))
        {
            damage = me->GetHealth() - me->CountPctFromMaxHealth(1);

            if (!TempestStorm)
            {
                TempestStorm = true;
                me->RemoveAllAuras();
                me->CastStop();
                events.Reset();
                me->AttackStop();
                me->SetReactState(REACT_PASSIVE);
                me->StopMoving();
                DoCast(me, SPELL_TEMPEST_STORM_TRANSFORM, true);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
            }
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (events.ExecuteEvent() == EVENT_CHAIN_LIGHTNING)
        {
            if (auto victim = me->getVictim())
                DoCast(victim, SPELL_CHAIN_LIGHTNING, false);

            events.ScheduleEvent(EVENT_CHAIN_LIGHTNING, 2000);
        }
        DoMeleeAttackIfReady();
    }
};

struct npc_cloud_burst : public ScriptedAI
{
    explicit npc_cloud_burst(Creature* creature) : ScriptedAI(creature)
    {
        uiTickCount = 0;
        me->SetInCombatWithZone();
        events.ScheduleEvent(EVENT_PERIODIC_CAST, 1000);
    }

    EventMap events;
    uint8 uiTickCount;

    void UpdateAI(uint32 diff) override
    {
        events.Update(diff);

        if (events.ExecuteEvent() == EVENT_PERIODIC_CAST)
        {
            ++uiTickCount;
            DoCast(SPELL_CLOUD_BURST);

            if (uiTickCount < 3)
                events.ScheduleEvent(EVENT_PERIODIC_CAST, 1000);
            else
                me->DespawnOrUnsummon(500);
        }
    }
};

class spell_wailing_winds : public SpellScript
{
    PrepareSpellScript(spell_wailing_winds);

    void RandomJump(SpellEffIndex effIndex)
    {
        PreventHitDefaultEffect(effIndex);
        Unit* target = GetHitUnit();
        if (!target)
            return;

        uint8 roll = urand(0, 1);
        float angle = frand(0, M_PI);
        float SpeedXY = frand(10.0f, 30.0f);
        float SpeedZ = frand(10.0f, 15.0f);
        float x, y;

        if (!roll)
        {
            target->GetPosition(x, y);
            target->SetOrientation(angle);
            target->KnockbackFrom(x, y, SpeedXY, SpeedZ);
        }
        else
        {
            float dist = frand(10.0f, 30.0f);
            target->GetNearPoint2D(x, y, dist, angle);
            target->GetMotionMaster()->MoveJump(x, y, FLOR_COORD_Z, SpeedXY, SpeedZ);
        }
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_wailing_winds::RandomJump, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

class spell_gathered_storms : public SpellScript
{
    PrepareSpellScript(spell_gathered_storms);

    void ApplyAura(SpellEffIndex effIndex)
    {
        Unit* target = GetHitUnit();
        if (!target)
            return;

        if (auto storm = target->ToCreature())
        {
            if (storm->GetEntry() == NPC_TEMPEST_STORM)
            {
                storm->RemoveAllAuras();
                storm->DespawnOrUnsummon(2000);
                PreventHitDefaultEffect(effIndex);
                return;
            }
        }
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_gathered_storms::ApplyAura, EFFECT_0, SPELL_EFFECT_APPLY_AURA);
    }
};

class achievement_headed_south : public AchievementCriteriaScript
{
    public:
        achievement_headed_south() : AchievementCriteriaScript("achievement_headed_south") {}

        bool OnCheck(Player* source, Unit* /*target*/)
        {
            if (source->GetMap()->GetDifficultyID() == DIFFICULTY_HEROIC)
                if (Aura* aura = source->GetAura(SPELL_LIGHTNING_CHARGE_AURA))
                    if (aura->GetStackAmount() >= 3)
                        return true;

            return false;
        }
};

void AddSC_boss_siamat()
{
    RegisterCreatureAI(boss_siamat);
    RegisterCreatureAI(npc_servant_of_siamat);
    RegisterCreatureAI(npc_siamat_minion);
    RegisterCreatureAI(npc_cloud_burst);
    RegisterSpellScript(spell_wailing_winds);
    RegisterSpellScript(spell_gathered_storms);
    new achievement_headed_south();
}