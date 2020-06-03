/*==============
==============*/

#include "gate_setting_sun.h"

enum eSpells
{
    // Commander Rimok
    SPELL_VISCOUS_FLUID_SUMMON      = 107078,
    SPELL_VISCOUS_FLUID_DMG_UP      = 107091,
    SPELL_VISCOUS_FLUID_DMG_DOWN    = 107122,
    SPELL_FRENZIED_ASSAULT          = 107120,

    // Add Generator
    SPELL_PERIODIC_SPAWN_SWARMER    = 115052,
    SPELL_PERIODIC_SPAWN_SABOTEUR   = 116621,

    // Saboteur
    SPELL_BOMBARD                   = 120559
};

enum eEvents
{
    EVENT_FRENZIED_ASSAULT  = 1,
    EVENT_VISCOUS_FLUID     = 2
};

Position const posGenerator[6] =
{
    { 1295.67f, 2324.82f, 382.075f, 0.0f },
    { 1300.45f, 2324.82f, 383.688f, 0.0f },
    { 1304.64f, 2324.52f, 384.497f, 0.0f },
    { 1275.06f, 2325.04f, 378.163f, 0.0f },
    { 1278.64f, 2324.42f, 376.938f, 0.0f },
    { 1289.14f, 2325.06f, 383.151f, 0.0f }
};

struct boss_commander_rimok : public BossAI
{
    explicit boss_commander_rimok(Creature* creature) : BossAI(creature, DATA_RIMOK) {}

    void Reset() override
    {
        _Reset();
        summons.DespawnAll();
        DespawnAllSummons();
    }

    void EnterCombat(Unit* /*who*/) override
    {
        _EnterCombat();
        SummonCreatures();
        events.RescheduleEvent(EVENT_FRENZIED_ASSAULT, urand(5000, 10000));
        events.RescheduleEvent(EVENT_VISCOUS_FLUID, urand(10000, 15000));
    }

    void SummonCreatures()
    {
        for (uint8 i = 0; i < 5; ++i)
            me->SummonCreature(59834, posGenerator[i], TEMPSUMMON_MANUAL_DESPAWN, 0);
    }

    void JustDied(Unit* /*who*/) override
    {
        _JustDied();
        DespawnAllSummons();
    }

    void DespawnAllSummons()
    {
        std::list<Creature*> list;
        list.clear();
        me->GetCreatureListWithEntryInGrid(list, 59835, 200.0f);
        me->GetCreatureListWithEntryInGrid(list, 60447, 200.0f);
        if (!list.empty())
            for (auto& cre : list)
                cre->DespawnOrUnsummon();
    }

    void JustSummoned(Creature* summoned) override
    {
        summons.Summon(summoned);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        switch (events.ExecuteEvent())
        {
        case EVENT_FRENZIED_ASSAULT:
            if (auto victim = me->getVictim())
                DoCast(victim, SPELL_FRENZIED_ASSAULT, false);

            events.RescheduleEvent(EVENT_FRENZIED_ASSAULT, urand(10000, 15000));
            break;
        case EVENT_VISCOUS_FLUID:
            Position pos;
            me->GetPosition(&pos);
            me->SummonCreature(56883, pos, TEMPSUMMON_TIMED_DESPAWN, 30000);
            events.RescheduleEvent(EVENT_VISCOUS_FLUID, urand(5000, 10000));
            break;
        }
        DoMeleeAttackIfReady();
    }
};

struct npc_krikthik_swarmer : public ScriptedAI
{
    explicit npc_krikthik_swarmer(Creature* creature) : ScriptedAI(creature) {}

    uint32 attackTimer;

    void Reset() override
    {
        attackTimer = 2000;
    }

    void UpdateAI(uint32 diff) override
    {
        if (attackTimer)
        {
            if (attackTimer <= diff)
            {
                DoZoneInCombat();
                if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                    AttackStart(target);

                attackTimer = 0;
            }
            else
                attackTimer -= diff;
        }
        DoMeleeAttackIfReady();
    }
};

struct npc_krikthik_saboteur : public ScriptedAI
{
    explicit npc_krikthik_saboteur(Creature* creature) : ScriptedAI(creature) {}

    uint32 attackTimer;
    uint32 checkTimer;

    void Reset() override
    {
        attackTimer = 2000;
        checkTimer = urand(17500, 22500);
    }

    void UpdateAI(uint32 diff) override
    {
        if (attackTimer)
        {
            if (attackTimer <= diff)
            {
                DoZoneInCombat();

                if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                    AttackStart(target);

                attackTimer = 0;
            }
            else
                attackTimer -= diff;
        }

        if (checkTimer <= diff)
        {
            DoCast(SPELL_BOMBARD);
            checkTimer = urand(7500, 12500);
        }
        else
            checkTimer -= diff;

        DoMeleeAttackIfReady();
    }
};

struct npc_add_generator : public ScriptedAI
{
    npc_add_generator(Creature* creature) : ScriptedAI(creature), summons(creature) {}

    SummonList summons;

    void Reset() override
    {
        me->RemoveAurasDueToSpell(SPELL_PERIODIC_SPAWN_SWARMER);
        me->RemoveAurasDueToSpell(SPELL_PERIODIC_SPAWN_SABOTEUR);
        summons.DespawnAll();
    }

    void IsSummonedBy(Unit* /*who*/) override
    {
        DoCast(me, SPELL_PERIODIC_SPAWN_SABOTEUR, true);
        DoCast(me, SPELL_PERIODIC_SPAWN_SWARMER, true);
    }

    void JustSummoned(Creature* summoned) override
    {
        summons.Summon(summoned);

        float x = me->GetPositionX();
        float y = me->GetPositionY() - 10;
        float z = me->GetMap()->GetHeight(x, y, 400.0f);
        summoned->GetMotionMaster()->MoveJump(x, y, z, 10, 20);

        if (summoned->GetEntry() == 63992)
            summoned->SetReactState(REACT_PASSIVE);
    }
};

struct npc_viscous_fluid : public ScriptedAI
{
    explicit npc_viscous_fluid(Creature* creature) : ScriptedAI(creature)
    {
        me->SetReactState(REACT_PASSIVE);
        me->SetDisplayId(11686);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_REMOVE_CLIENT_CONTROL);
    }

    void Reset() override
    {
        DoCast(107092);
        DoCast(107088);
    }
};

class spell_rimok_saboteur_bombard : public AuraScript
{
    PrepareAuraScript(spell_rimok_saboteur_bombard);

    void OnPeriodic(AuraEffect const* /*aurEff*/)
    {
        PreventDefaultAction();

        if (Unit* caster = GetCaster())
        {
            if (InstanceScript* instance = caster->GetInstanceScript())
            {
                Map::PlayerList const &PlayerList = instance->instance->GetPlayers();

                if (PlayerList.isEmpty())
                    return;

                Map::PlayerList::const_iterator it = PlayerList.begin();
                for (uint8 i = 0; i < urand(0, PlayerList.getSize() - 1); ++i, ++it);

                if (it == PlayerList.end())
                    return;

                if (Player* player = it->getSource())
                    caster->CastSpell(player, GetSpellInfo()->Effects[0]->TriggerSpell, true);
            }
        }
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_rimok_saboteur_bombard::OnPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
    }
};

void AddSC_boss_commander_rimok()
{
    RegisterCreatureAI(boss_commander_rimok);
    RegisterCreatureAI(npc_krikthik_swarmer);
    RegisterCreatureAI(npc_krikthik_saboteur);
    RegisterCreatureAI(npc_add_generator);
    RegisterCreatureAI(npc_viscous_fluid);
    RegisterAuraScript(spell_rimok_saboteur_bombard);
}
