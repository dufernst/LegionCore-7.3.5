#include "grim_batol.h"
#include "Vehicle.h"

enum ScriptTexts
{
    //drahga
    SAY_AGGRO   = 0,
    SAY_KILL    = 2,
    SAY_DEATH   = 3,
    SAY_ADDS    = 4,
    SAY_VALIONA = 5,

    //valiona
    SAY_ENTER   = 0,
    SAY_LEAVE   = 1
};

enum Spells
{
    SPELL_BURNING_SHADOWBOLT        = 75245,
    SPELL_INVOCATION_OF_FLAME       = 75222,
    SPELL_INVOCATION_OF_FLAME_SUMM  = 75218,
    SPELL_FLAMING_FIXATE            = 82850,
    SPELL_INVOKED_FLAME             = 75235,
    SPELL_SUPERNOVA                 = 75238,
    SPELL_TWILIGHT_PROTECTION       = 76303,
    SPELL_TWILIGHT_SHIFT            = 75328,
    SPELL_SHREDDING_SWIPE           = 75271,
    SPELL_SEEPING_TWILIGHT_DUMMY    = 75318,
    SPELL_SEEPING_TWILIGHT          = 75274,
    SPELL_SEEPING_TWILIGHT_DMG      = 75317,
    SPELL_VALIONAS_FLAME            = 75321,
    SPELL_DEVOURING_FLAMES          = 90950
};

enum Adds
{
    NPC_INVOCATION_OF_FLAME_STALKER     = 40355,
    NPC_INVOKED_FLAMING_SPIRIT          = 40357,
    NPC_VALIONA                         = 40320,
    NPC_SEEPING_TWILIGHT                = 40365,
    NPC_DEVOURING_FLAMES                = 48798
};

enum Events
{
    EVENT_BURNING_SHADOWBOLT    = 1,
    EVENT_INVOCATION_OF_FLAME   = 2,
    EVENT_SELECT_TARGET         = 3,
    EVENT_VALIONAS_FLAME        = 4,
    EVENT_SHREDDING_SWIPE       = 5
};

const Position drahgavalionaPos[2] =
{
    {-391.38f, -678.52f, 275.56f, 3.68f},
    {-431.79f, -697.17f, 268.62f, 3.45f}
};

struct boss_drahga_shadowburner : public BossAI
{
    explicit boss_drahga_shadowburner(Creature* creature) : BossAI(creature, DATA_DRAHGA_SHADOWBURNER) {}

    bool stage;
    bool twophasecomplete;

    void Reset() override
    {
        _Reset();
        me->ExitVehicle();
        me->SetReactState(REACT_AGGRESSIVE);
        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        me->RemoveAllAuras();
        summons.DespawnAll();
        events.Reset();
        stage = false;
        twophasecomplete = false;
    }

    void SpellHit(Unit* /*caster*/, SpellInfo const* spell) override
    {
        if (me->GetCurrentSpell(CURRENT_GENERIC_SPELL))
            if (me->GetCurrentSpell(CURRENT_GENERIC_SPELL)->m_spellInfo->Id == SPELL_BURNING_SHADOWBOLT)
                for (uint8 i = 0; i < 3; ++i)
                    if (spell->Effects[i]->Effect == SPELL_EFFECT_INTERRUPT_CAST)
                        me->InterruptSpell(CURRENT_GENERIC_SPELL);
    }

    void JustSummoned(Creature* summon) override
    {
        summons.Summon(summon);
    }

    void DamageTaken(Unit* /*attacker*/, uint32 &damage, DamageEffectType dmgType) override
    {
        if (me->HealthBelowPct(30) && !stage)
        {
            stage = true;
            DoCast(SPELL_TWILIGHT_PROTECTION);
            Talk(SAY_VALIONA);
            me->SetReactState(REACT_PASSIVE);
            me->AttackStop();
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);

            if (auto valiona = me->SummonCreature(NPC_VALIONA, drahgavalionaPos[0], TEMPSUMMON_MANUAL_DESPAWN))
                DoCast(valiona, 46598, true);
        }

        if (damage >= me->GetHealth() && !twophasecomplete)
            damage = 0;
    }

    void SummonedCreatureDespawn(Creature* summon)
    {
        summons.Despawn(summon);
    }

    void EnterCombat(Unit* /*who*/) override
    {
        _EnterCombat();
        Talk(SAY_AGGRO);
        events.RescheduleEvent(EVENT_BURNING_SHADOWBOLT, urand(2000, 5000));
        events.RescheduleEvent(EVENT_INVOCATION_OF_FLAME, 10000);
    }

    void JustDied(Unit* /*killer*/) override
    {
        _JustDied();
        Talk(SAY_DEATH);
        summons.DespawnAll();
    }

    void KilledUnit(Unit* victim) override
    {
        if (!victim)
            return;

        if (victim->IsPlayer())
            Talk(SAY_KILL);
    }

    void DoAction(int32 const action) override
    {
        switch (action)
        {
        case ACTION_1:
            me->RemoveAurasDueToSpell(SPELL_TWILIGHT_PROTECTION);
            me->SetReactState(REACT_AGGRESSIVE);
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            twophasecomplete = true;
            break;
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        if (CheckHomeDistToEvade(diff, 60.0f))
            return;

        events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        if (stage)
            return;

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_BURNING_SHADOWBOLT:
                DoCast(SPELL_BURNING_SHADOWBOLT);
                events.RescheduleEvent(EVENT_BURNING_SHADOWBOLT, urand(8000, 15000));
                break;
            case EVENT_INVOCATION_OF_FLAME:
                if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                    DoCast(target, SPELL_INVOCATION_OF_FLAME_SUMM, false);

                Talk(SAY_ADDS);
                events.RescheduleEvent(EVENT_INVOCATION_OF_FLAME, 20000);
                break;
            }
        }

        if (!me->GetVehicle())
            DoMeleeAttackIfReady();
    }
};

struct npc_drahga_valiona : public ScriptedAI
{
    explicit npc_drahga_valiona(Creature* creature) : ScriptedAI(creature), summons(me)
    {
        instance = creature->GetInstanceScript();
    }

    InstanceScript* instance;
    EventMap events;
    SummonList summons;
    bool stage;

    void Reset() override
    {
        summons.DespawnAll();
        events.Reset();
        stage = false;
    }

    void JustSummoned(Creature* summon) override
    {
        summons.Summon(summon);
    }

    void SummonedCreatureDespawn(Creature* summon) override
    {
        summons.Despawn(summon);
    }

    void IsSummonedBy(Unit* /*owner*/) override
    {
        me->SetReactState(REACT_PASSIVE);
        me->SetCanFly(true);
        me->GetMotionMaster()->MovePoint(1001, drahgavalionaPos[1]);
    }

    void EnterCombat(Unit* /*who*/) override
    {
        events.RescheduleEvent(EVENT_VALIONAS_FLAME, urand(10000, 15000));
        events.RescheduleEvent(EVENT_SHREDDING_SWIPE, urand(8000, 10000));
    }

    void MovementInform(uint32 type, uint32 id) override
    {
        if (type == POINT_MOTION_TYPE)
        {
            switch (id)
            {
            case 1001:
                me->SetReactState(REACT_AGGRESSIVE);
                me->SetCanFly(false);
                Talk(SAY_ENTER);
                break;
            case 1002:
                me->DespawnOrUnsummon();
                break;
            }
        }
    }

    void EnterEvadeMode() override
    {
        if (me->GetVehicleKit())
            me->GetVehicleKit()->RemoveAllPassengers();

        me->DespawnOrUnsummon();
    }

    void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType dmgType) override
    {
        if (damage >= me->GetHealth())
            damage = 0;

        if (me->HealthBelowPct(30) && !stage)
        {
            stage = true;
            events.Reset();
            DoCast(SPELL_TWILIGHT_SHIFT);
            me->SetReactState(REACT_PASSIVE);
            Talk(SAY_LEAVE);

            if (me->GetVehicleKit())
                me->GetVehicleKit()->RemoveAllPassengers();

            me->SetCanFly(true);
            me->GetMotionMaster()->MovePoint(1002, drahgavalionaPos[0]);

            if (auto drahga = me->GetAnyOwner())
                drahga->GetAI()->DoAction(ACTION_1);
        }
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
            case EVENT_VALIONAS_FLAME:
                if (IsHeroic())
                    DoCast(SPELL_DEVOURING_FLAMES);
                else
                    DoCast(SPELL_VALIONAS_FLAME);

                events.RescheduleEvent(EVENT_VALIONAS_FLAME, urand(15000, 22000));
                break;
            case EVENT_SHREDDING_SWIPE:
                if (auto victim = me->getVictim())
                    DoCast(victim, SPELL_SHREDDING_SWIPE, false);

                events.RescheduleEvent(EVENT_SHREDDING_SWIPE, urand(20000, 22000));
                break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

struct npc_invocation_of_flame_stalker : public ScriptedAI
{
    explicit npc_invocation_of_flame_stalker(Creature* creature) : ScriptedAI(creature)
    {
        instance = creature->GetInstanceScript();
    }

    InstanceScript* instance;

    void Reset() override
    {
        DoCast(SPELL_INVOCATION_OF_FLAME);
    }

    void JustSummoned(Creature* summon) override
    {
        switch (summon->GetEntry())
        {
        case NPC_INVOKED_FLAMING_SPIRIT:
            if (auto drahga = Unit::GetCreature(*me, instance->GetGuidData(DATA_DRAHGA_SHADOWBURNER)))
            {
                if (auto target = drahga->GetAI()->SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                {
                    summon->AI()->AttackStart(target);
                    summon->CastSpell(target, SPELL_FLAMING_FIXATE, false);
                }
            }
            break;
        }
    }
};

struct npc_invoked_flaming_spirit : public ScriptedAI
{
    explicit npc_invoked_flaming_spirit(Creature* creature) : ScriptedAI(creature)
    {
        me->SetSpeed(MOVE_RUN, 0.8f);
        me->SetSpeed(MOVE_WALK, 0.8f);
        instance = creature->GetInstanceScript();
    }

    InstanceScript* instance;

    void Reset() override
    {
        DoCast(SPELL_INVOKED_FLAME);
    }

    void JustDied(Unit* /*target*/) override
    {
        me->DespawnOrUnsummon();
    }

    void SpellFinishCast(SpellInfo const* spell) override
    {
        switch (spell->Id)
        {
        case SPELL_FLAMING_FIXATE:
            me->ClearUnitState(UNIT_STATE_CASTING);
            break;
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (!instance)
            return;

        if (instance->GetData(DATA_DRAHGA_SHADOWBURNER) != IN_PROGRESS)
            me->DespawnOrUnsummon();

        DoMeleeAttackIfReady();
    }
};

struct npc_seeping_twilight : public ScriptedAI
{
    explicit npc_seeping_twilight(Creature* creature) : ScriptedAI(creature) {}

    void Reset() override
    {
        DoCast(SPELL_SEEPING_TWILIGHT_DUMMY);
        DoCast(SPELL_SEEPING_TWILIGHT);
    }
};

class spell_drahga_supernova : public SpellScript
{
    PrepareSpellScript(spell_drahga_supernova);

    void HandleScriptEffect(SpellEffIndex effIndex)
    {
        PreventHitDefaultEffect(effIndex);

        Unit* caster = GetCaster();
        if (!caster)
            return;

        caster->ToCreature()->DespawnOrUnsummon(500);
    }

    void Register()
    {
        OnEffectHitTarget += SpellEffectFn(spell_drahga_supernova::HandleScriptEffect, EFFECT_1, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

void AddSC_boss_drahga_shadowburner()
{
    RegisterCreatureAI(boss_drahga_shadowburner);
    RegisterCreatureAI(npc_invocation_of_flame_stalker);
    RegisterCreatureAI(npc_invoked_flaming_spirit);
    RegisterCreatureAI(npc_drahga_valiona);
    RegisterCreatureAI(npc_seeping_twilight);
    RegisterSpellScript(spell_drahga_supernova);
}