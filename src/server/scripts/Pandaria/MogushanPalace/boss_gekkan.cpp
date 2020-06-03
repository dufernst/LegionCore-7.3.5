/*===============
================*/

#include "mogu_shan_palace.h"

enum eActions
{
    ACTION_ENTOURAGE_DIED = 1
};

enum eEvents
{
    EVENT_RECKLESS_INSPIRATION = 1
};

enum eTalks
{
    TALK_INTRO,
    TALK_KILLING,
    TALK_SPELL,
    TALK_AGGRO,
    TALK_DEATH
};

struct boss_gekkan : public BossAI
{
    explicit boss_gekkan(Creature* creature) : BossAI(creature, DATA_GEKKAN) {}

    void Reset() override {}

    void EnterCombat(Unit* /*unit*/) override
    {
        events.RescheduleEvent(EVENT_RECKLESS_INSPIRATION, 3000);
        Talk(TALK_AGGRO);
        instance->SetData(DATA_GEKKAN_ADDS, 1);
    }


    void JustDied(Unit* /*who*/) override
    {
        instance->DoUpdateAchievementCriteria(CRITERIA_TYPE_BE_SPELL_TARGET2, SPELL_SAUROK_ACHIEV_AURA, 0, 0, me);
        Talk(TALK_DEATH);
    }

    void KilledUnit(Unit* u) override
    {
        if (!u && !u->IsPlayer())
            return;

        Talk(TALK_KILLING);
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
            case EVENT_RECKLESS_INSPIRATION:
                if (auto ironhide = me->FindNearestCreature(CREATURE_GLINTROK_IRONHIDE, 20.0f, true))
                {
                    Talk(TALK_SPELL);
                    DoCast(ironhide, 118988, false);
                }
                else if (auto skulker = me->FindNearestCreature(CREATURE_GLINTROK_SKULKER, 20.0f, true))
                {
                    Talk(TALK_SPELL);
                    DoCast(skulker, 118988, false);
                }
                else if (auto oracle = me->FindNearestCreature(CREATURE_GLINTROK_ORACLE, 20.0f, true))
                {
                    Talk(TALK_SPELL);
                    DoCast(oracle, 118988, false);
                }
                else if (auto hexxer = me->FindNearestCreature(CREATURE_GLINTROK_HEXXER, 20.0f, true))
                {
                    Talk(TALK_SPELL);
                    DoCast(hexxer, 118988, false);
                }
                else
                    me->AddAura(118988, me);

                events.RescheduleEvent(EVENT_RECKLESS_INSPIRATION, 24000);
                break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

struct mob_glintrok_skulker : public ScriptedAI
{
    explicit mob_glintrok_skulker(Creature* creature) : ScriptedAI(creature)
    {
        instance = me->GetInstanceScript();
    }

    EventMap events;
    InstanceScript* instance;

    void Reset() override
    {
        me->RemoveAllAuras();
    }

    void EnterCombat(Unit* /*unit*/) override
    {
        events.RescheduleEvent(EVENT_1, 2000);
        instance->SetData(DATA_GEKKAN_ADDS, 1);
    }

    void EnterEvadeMode() override
    {
        ScriptedAI::EnterEvadeMode();
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_1:
                if (auto victim = me->getVictim())
                    DoCast(victim, 118963, false);

                events.RescheduleEvent(EVENT_1, 7000);
                break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

struct mob_glintrok_ironhide : public ScriptedAI
{
    explicit mob_glintrok_ironhide(Creature* creature) : ScriptedAI(creature)
    {
        instance = me->GetInstanceScript();
    }

    EventMap events;
    InstanceScript* instance;

    void Reset() override
    {
        me->RemoveAllAuras();
    }

    void EnterCombat(Unit* /*unit*/) override
    {
        events.RescheduleEvent(EVENT_1, 2000);
        instance->SetData(DATA_GEKKAN_ADDS, 1);
    }

    void EnterEvadeMode() override
    {
        me->RemoveAllAuras();
        ScriptedAI::EnterEvadeMode();
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_1:
                DoCast(118958);
                events.RescheduleEvent(EVENT_1, 15000);
                break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

struct mob_glintrok_oracle : public ScriptedAI
{
    explicit mob_glintrok_oracle(Creature* creature) : ScriptedAI(creature)
    {
        instance = me->GetInstanceScript();
    }

    EventMap events;
    InstanceScript* instance;

    void Reset() override
    {
        me->RemoveAllAuras();
    }

    void EnterEvadeMode() override
    {
        me->RemoveAllAuras();
        ScriptedAI::EnterEvadeMode();
    }

    void EnterCombat(Unit* /*unit*/) override
    {
        events.RescheduleEvent(EVENT_1, 2000);
        events.RescheduleEvent(EVENT_2, 4000);
        instance->SetData(DATA_GEKKAN_ADDS, 1);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_1:
                DoCast(118940);
                events.RescheduleEvent(EVENT_1, 25000);
                break;
            case EVENT_2:
                if (auto victim = me->getVictim())
                    DoCast(victim, 118936, false);

                events.RescheduleEvent(EVENT_2, 7000);
                break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

struct mob_glintrok_hexxer : public ScriptedAI
{
    explicit mob_glintrok_hexxer(Creature* creature) : ScriptedAI(creature)
    {
        instance = me->GetInstanceScript();
    }

    EventMap events;
    InstanceScript* instance;

    void Reset() override
    {
        me->RemoveAllAuras();
    }

    void EnterCombat(Unit* /*unit*/) override
    {
        events.RescheduleEvent(EVENT_1, 2000);
        events.RescheduleEvent(EVENT_2, 4000);
        instance->SetData(DATA_GEKKAN_ADDS, 1);
    }

    void EnterEvadeMode() override
    {
        me->RemoveAllAuras();
        ScriptedAI::EnterEvadeMode();
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_1:
                if (auto victim = me->getVictim())
                    DoCast(victim, 118903, false);

                events.RescheduleEvent(EVENT_1, 20000);
                break;
            case EVENT_2:
                if (auto victim = me->getVictim())
                    DoCast(victim, 118917, false);

                events.RescheduleEvent(EVENT_2, 5000);
                break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

void AddSC_boss_gekkan()
{
    RegisterCreatureAI(boss_gekkan);
    RegisterCreatureAI(mob_glintrok_hexxer);
    RegisterCreatureAI(mob_glintrok_skulker);
    RegisterCreatureAI(mob_glintrok_oracle);
    RegisterCreatureAI(mob_glintrok_ironhide);
}