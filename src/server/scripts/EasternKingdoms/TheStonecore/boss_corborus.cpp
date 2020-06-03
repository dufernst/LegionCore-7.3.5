#include "the_stonecore.h"

enum Spells
{
    SPELL_CRYSTAL_BARRAGE       = 86881,
    SPELL_CRYSTAL_BARRAGE_SUM   = 92012,
    SPELL_CRYSTAL_SHARD_DMG     = 92122,
    SPELL_DUMPENING_WAVE        = 82415,
    SPELL_SUBMERGE              = 53421,
    SPELL_THRASHING_CHARGE_DMG  = 81828,
    SPELL_THRASHING_CHARGE_SUM  = 81816,
    SPELL_THRASHING_CHARGE_DUM  = 81801,
    SPELL_ROCK_BORE             = 80028             
};

enum Events
{
    EVENT_CRYSTAL_BARRAGE       = 1,
    EVENT_DUMPENING_WAVE        = 2,
    EVENT_MERGE                 = 3,
    EVENT_SUBMERGE              = 4,
    EVENT_CRYSTAL_BARRAGE_H     = 5,
    EVENT_THRASHING_CHARGE      = 6,
    EVENT_ROCK_BORER            = 7,
    EVENT_ROCK_BORE             = 8,
    EVENT_CRYSTAL_SHARD_MOVE    = 9
};

enum Adds
{
    NPC_ROCK_BORER          = 43917,
    NPC_CRYSTAL_SHARD       = 49267,
    NPC_THRASHING_CHARGE    = 43743
};

struct boss_corborus : public BossAI
{
    explicit boss_corborus(Creature* creature) : BossAI(creature, DATA_CORBORUS) {}

    uint8 stage;
    Position barragePos;

    void Reset() override
    {
        _Reset();

        stage = 0;
        me->RemoveAurasDueToSpell(SPELL_SUBMERGE);
        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        events.Reset();
        summons.DespawnAll();
    }

    void EnterCombat(Unit* /*who*/) override
    {
        events.RescheduleEvent(EVENT_DUMPENING_WAVE, 5000);
        events.RescheduleEvent(EVENT_CRYSTAL_BARRAGE, 7000);
        events.RescheduleEvent(EVENT_SUBMERGE, 30000);
        instance->SetBossState(DATA_CORBORUS, IN_PROGRESS);
    }

    void JustSummoned(Creature* summon) override
    {
        summons.Summon(summon);

        switch (summon->GetEntry())
        {
        case NPC_THRASHING_CHARGE:
            summon->CastSpell(summon, SPELL_THRASHING_CHARGE_DUM, false);
            break;
        case NPC_ROCK_BORER:
            if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
            {
                summon->AddThreat(target, 10.0f);
                summon->Attack(target, true);
                summon->GetMotionMaster()->MoveChase(target);
            }
            break;
        case NPC_CRYSTAL_SHARD:
            if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
            {
                summon->AddThreat(target, 10.0f);
                summon->Attack(target, true);
                summon->GetMotionMaster()->MoveChase(target);
            }
            break;
        }
    }

    void JustDied(Unit* /*killer*/) override
    {
        _JustDied();
        summons.DespawnAll();
    }

    void SummonedCreatureDespawn(Creature* summon) override
    {
        summons.Despawn(summon);
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
            case EVENT_DUMPENING_WAVE:
                DoCast(SPELL_DUMPENING_WAVE);
                events.RescheduleEvent(EVENT_DUMPENING_WAVE, 10000);
                break;
            case EVENT_CRYSTAL_BARRAGE:
                if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                {
                    DoCast(target, SPELL_CRYSTAL_BARRAGE, false);

                    if (IsHeroic())
                    {
                        target->GetPosition(&barragePos);
                        events.RescheduleEvent(EVENT_CRYSTAL_BARRAGE_H, 4000);
                    }
                }

                events.RescheduleEvent(EVENT_CRYSTAL_BARRAGE, 15000);
                break;
            case EVENT_CRYSTAL_BARRAGE_H:
                for (uint8 i = 0; i < 5; i++)
                    me->CastSpell(barragePos.GetPositionX(), barragePos.GetPositionY(), barragePos.GetPositionZ(), SPELL_CRYSTAL_BARRAGE_SUM, true);
                break;
            case EVENT_SUBMERGE:
                me->RemoveAllAuras();
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                DoCast(SPELL_SUBMERGE);
                events.Reset();
                events.RescheduleEvent(EVENT_THRASHING_CHARGE, 4000);
                events.RescheduleEvent(EVENT_ROCK_BORER, 3000);
                events.RescheduleEvent(EVENT_MERGE, 20000);
                break;
            case EVENT_MERGE:
                me->RemoveAurasDueToSpell(SPELL_SUBMERGE);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                events.Reset();
                events.RescheduleEvent(EVENT_DUMPENING_WAVE, 5000);
                events.RescheduleEvent(EVENT_CRYSTAL_BARRAGE, 7000);
                events.RescheduleEvent(EVENT_SUBMERGE, 30000);
                break;
            case EVENT_THRASHING_CHARGE:
                if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                    me->CastSpell(target, SPELL_THRASHING_CHARGE_SUM, true);

                events.RescheduleEvent(EVENT_THRASHING_CHARGE, 4000);
                break;
            case EVENT_ROCK_BORER:
                if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                    me->SummonCreature(NPC_ROCK_BORER, target->GetPositionX() + urand(3, 5), target->GetPositionY() + urand(3, 5), target->GetPositionZ(), 0.0f);

                events.RescheduleEvent(EVENT_ROCK_BORER, 3000);
                break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

struct npc_rock_borer : public ScriptedAI
{
    explicit npc_rock_borer(Creature* creature) : ScriptedAI(creature) {}

    EventMap events;

    void Reset() override
    {
        events.Reset();
    }

    void EnterCombat(Unit* /*who*/) override
    {
        events.RescheduleEvent(EVENT_ROCK_BORE, 2000);
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
            case EVENT_ROCK_BORE:
                if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                    DoCast(target, SPELL_ROCK_BORE, false);

                events.RescheduleEvent(EVENT_ROCK_BORE, 5000);
                return;
            }
        }
        DoMeleeAttackIfReady();
    }
};

void AddSC_boss_corborus()
{
    RegisterCreatureAI(boss_corborus);
    RegisterCreatureAI(npc_rock_borer);
}