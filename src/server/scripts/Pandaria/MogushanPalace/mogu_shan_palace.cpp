#include "mogu_shan_palace.h"

enum otherEvents
{
    EVENT_RUN_FOREST_RUN = 1,
    EVENT_GEM_LIMIT_TIME = 2
};

struct npc_jade_quilen : public ScriptedAI
{
    npc_jade_quilen(Creature* creature) : ScriptedAI(creature)
    {
        instance = creature->GetInstanceScript();
        OneClick = false;
    }

    InstanceScript* instance;
    bool OneClick;

    void OnSpellClick(Unit* /*clicker*/) override
    {
        if (instance && !OneClick)
        {
            OneClick = true;
            uint32 JadeCount = instance->GetData(TYPE_JADECOUNT) + 1;
            instance->SetData(TYPE_JADECOUNT, JadeCount);
            me->DespawnOrUnsummon();
        }
    }
};

struct npc_glintrok_scout : public ScriptedAI
{
    npc_glintrok_scout(Creature* creature) : ScriptedAI(creature)
    {
        instance = creature->GetInstanceScript();
    }

    InstanceScript* instance;
    EventMap events;

    void Reset() override
    {
        me->SetReactState(REACT_AGGRESSIVE);
    }

    void EnterCombat(Unit* /*who*/) override
    {
        if (instance)
        {
            Talk(0);
            DoCast(SPELL_GLINTROK_SCOUT_WARNING);
            events.RescheduleEvent(EVENT_RUN_FOREST_RUN, 500);
        }
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
            case EVENT_RUN_FOREST_RUN:
                me->SetReactState(REACT_PASSIVE);
                me->AttackStop();
                me->GetMotionMaster()->Clear();
                me->GetMotionMaster()->MovePoint(1, otherPos[1]);
                break;
            }
        }
        DoMeleeAttackIfReady();
    }

    void MovementInform(uint32 type, uint32 id) override
    {
        if (type == POINT_MOTION_TYPE)
        {
            if (id == 1)
                me->DespawnOrUnsummon();
        }
    }
};

struct npc_faintly_glowing_gem : public ScriptedAI
{
    npc_faintly_glowing_gem(Creature* creature) : ScriptedAI(creature)
    {
        instance = creature->GetInstanceScript();
        me->SetReactState(REACT_PASSIVE);
        OneClick = false;
    }

    InstanceScript* instance;
    EventMap events;
    bool OneClick;

    void OnSpellClick(Unit* /*clicker*/) override
    {
        if (instance && !OneClick)
        {
            OneClick = true;
            DoCast(SPELL_TURN_OFF_BLADES);
            uint32 GemCount = instance->GetData(TYPE_GEMCOUNT) + 1;
            instance->SetData(TYPE_GEMCOUNT, GemCount);
            events.RescheduleEvent(EVENT_GEM_LIMIT_TIME, 5000);
            if (GemCount == 2)
            {
                DoCast(SPELL_SECRET_DEFENSE_MECHANISM);
                events.CancelEvent(EVENT_GEM_LIMIT_TIME);
            }
        }
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
            case EVENT_GEM_LIMIT_TIME:
                uint32 GemCount = instance->GetData(TYPE_GEMCOUNT) - 1;
                instance->SetData(TYPE_GEMCOUNT, GemCount);
                break;
            }
        }
    }
};

void AddSC_mogu_shan_palace()
{
    RegisterCreatureAI(npc_jade_quilen);
    RegisterCreatureAI(npc_glintrok_scout);
    RegisterCreatureAI(npc_faintly_glowing_gem);
}