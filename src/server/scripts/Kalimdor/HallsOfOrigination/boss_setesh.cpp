#include "halls_of_origination.h"

enum Texts
{
    SAY_DEATH   = 0,
    SAY_AGGRO   = 1,
    SAY_KILL    = 2
};

enum Spells
{
    // Setesh
    SPELL_CHAOS_BOLT            = 77069,

    // Void Sentinel
    SPELL_VOID_BARRIER          = 76959,
    SPELL_CHARGED_FISTS         = 77238,

    // Void Seeker
    SPELL_ANTI_MAGIC_PRISON     = 76903,
    SPELL_SHADOW_BOLT_VOLLEY    = 76146
};

enum NPCs
{
    NPC_VOID_SENTINEL       = 41208,
    NPC_VOID_SEEKER         = 41371,
    NPC_VOID_WURM           = 41374,
    NPC_CHAOS_PORTAL        = 41055,
    NPC_REIGN_OF_CHAOS      = 41168,
    NPC_VOID_LORD           = 41364
};

enum Events
{
    EVENT_CHAOS_BOLT            = 1,
    EVENT_SUMMON_CHAOS_PORTAL   = 2,
    EVENT_SUMMON_6              = 3,
    EVENT_SUMMON_10             = 4,
    EVENT_SUMMON_12             = 5,
    EVENT_SUMMON_15             = 6,
    EVENT_ANTI_MAGIC_PRISON     = 7,
    EVENT_SHADOW_BOLT_VOLLEY    = 8,
    EVENT_VOID_BARRIER          = 9,
    EVENT_CHARGED_FISTS         = 10,
    EVENT_MOVE                  = 11
};

enum SeteshSummonTypes
{
    SETESH_SUMMON_WURM      = 1,
    SETESH_SUMMON_SENTINEL  = 2,
    SETESH_SUMMON_SEEKER    = 3
};

const Position movepos[9] =
{
    {-481.55f, 14.15f, 343.92f, 2.07f},
    {-490.31f, 25.55f, 343.93f, 2.56f},
    {-508.35f, 30.95f, 343.94f, 3.08f},
    {-524.62f, 30.30f, 343.93f, 3.35f},
    {-534.63f, 22.76f, 343.92f, 4.08f},
    {-539.40f, 9.067f, 343.92f, 4.67f},
    {-537.78f, -3.28f, 343.92f, 5.11f},
    {-528.39f, -16.43f, 343.93f, 5.85f},
    {-513.85f, -19.04f, 343.93f, 6.15f}
};

struct boss_setesh : public BossAI
{
    explicit boss_setesh(Creature* creature) : BossAI(creature, DATA_SETESH)
    {
        me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
        me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
        me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_STUN, true);
        me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FEAR, true);
        me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_ROOT, true);
        me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FREEZE, true);
        me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_POLYMORPH, true);
        me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_HORROR, true);
        me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_SAPPED, true);
        me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_CHARM, true);
        me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_DISORIENTED, true);
        me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_CONFUSE, true);
    }

    void Reset() override
    {
        _Reset();
    }

    void EnterCombat(Unit* /*who*/) override
    {
        Talk(SAY_AGGRO);
        events.ScheduleEvent(EVENT_CHAOS_BOLT, 10000);
        events.ScheduleEvent(EVENT_SUMMON_CHAOS_PORTAL, 20000);
        events.ScheduleEvent(EVENT_MOVE, 2000);

        me->SetReactState(REACT_PASSIVE);

        DoZoneInCombat();
        instance->SetBossState(DATA_SETESH, IN_PROGRESS);
    }

    void MovementInform(uint32 type, uint32 id) override
    {
        if (type == POINT_MOTION_TYPE)
        {
            switch (id)
            {
            case 1:
                events.ScheduleEvent(EVENT_MOVE, 1000);
                break;
            }
        }
    }

    void KilledUnit(Unit* /*who*/) override
    {
        Talk(SAY_KILL);
    }

    void JustDied(Unit* /*who*/)
    {
        _JustDied();

        Talk(SAY_DEATH);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_CHAOS_BOLT:
                if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                    DoCast(target, SPELL_CHAOS_BOLT, false);

                events.ScheduleEvent(EVENT_CHAOS_BOLT, urand(5000, 10000));
                break;
            case EVENT_SUMMON_CHAOS_PORTAL:
                me->SummonCreature(NPC_CHAOS_PORTAL, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation(), IsHeroic() ? TEMPSUMMON_DEAD_DESPAWN : TEMPSUMMON_TIMED_DESPAWN, IsHeroic() ? 0 : 35000);
                events.ScheduleEvent(EVENT_SUMMON_CHAOS_PORTAL, urand(40000, 45000));
                break;
            case EVENT_MOVE:
                me->GetMotionMaster()->MovePoint(1, movepos[urand(0, 8)]);
                break;
            }
        }
    }
};

struct npc_setesh_chaos_portal : public ScriptedAI
{
    explicit npc_setesh_chaos_portal(Creature* creature) : ScriptedAI(creature)
    {
        instance = creature->GetInstanceScript();
    }

    InstanceScript* instance;
    EventMap events;

    void SeteshSummon(SeteshSummonTypes type)
    {
        if (!instance)
            return;

        if (auto setesh = instance->instance->GetCreature(instance->GetGuidData(DATA_SETESH)))
        {
            switch (type)
            {
            case SETESH_SUMMON_WURM:
                setesh->SummonCreature(NPC_VOID_WURM, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation());
                setesh->SummonCreature(NPC_VOID_WURM, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation());
                break;
            case SETESH_SUMMON_SENTINEL:
                setesh->SummonCreature(NPC_VOID_SENTINEL, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation());
                break;
            case SETESH_SUMMON_SEEKER:
                setesh->SummonCreature(NPC_VOID_SEEKER, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation());
                break;
            }
        }
    }

    void Reset() override
    {
        me->SetReactState(REACT_PASSIVE);

        if (!IsHeroic())
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_PACIFIED);
            SeteshSummon(SETESH_SUMMON_WURM);
            events.ScheduleEvent(EVENT_SUMMON_12, 12000);
        }
        else
        {
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_PACIFIED);
            SeteshSummon(SETESH_SUMMON_SENTINEL);
            events.ScheduleEvent(EVENT_SUMMON_6, 12000);
        }
    }

    void UpdateAI(uint32 diff) override
    {
        events.Update(diff);

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_SUMMON_6:
                SeteshSummon(SETESH_SUMMON_WURM);
                events.ScheduleEvent(EVENT_SUMMON_10, 10000);
                break;
            case EVENT_SUMMON_10:
                SeteshSummon(SETESH_SUMMON_SEEKER);
                events.ScheduleEvent(EVENT_SUMMON_15, 15000);
                break;
            case EVENT_SUMMON_12:
                SeteshSummon(SETESH_SUMMON_SEEKER);
                events.ScheduleEvent(EVENT_SUMMON_15, 15000);
                break;
            case EVENT_SUMMON_15:
                if (IsHeroic())
                {
                    if (urand(0, 1))
                        SeteshSummon(SETESH_SUMMON_WURM);
                    else
                        SeteshSummon(SETESH_SUMMON_SENTINEL);

                    events.ScheduleEvent(EVENT_SUMMON_15, 15000);
                }
                else
                {
                    SeteshSummon(SETESH_SUMMON_SENTINEL);
                    me->DespawnOrUnsummon();
                }
                break;
            }
        }
    }
};

struct npc_setesh_void_sentinel : public ScriptedAI
{
    explicit npc_setesh_void_sentinel(Creature* creature) : ScriptedAI(creature)
    {
        instance = creature->GetInstanceScript();
    }

    InstanceScript* instance;
    EventMap events;

    void Reset() override
    {
        events.Reset();
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_CHARGED_FISTS:
                DoCast(SPELL_CHARGED_FISTS);
                events.ScheduleEvent(EVENT_CHARGED_FISTS, urand(18000, 22000));
                break;
            case EVENT_VOID_BARRIER:
                DoCast(SPELL_VOID_BARRIER);
                break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

struct npc_setesh_void_seeker : public ScriptedAI
{
    explicit npc_setesh_void_seeker(Creature* creature) : ScriptedAI(creature)
    {
        instance = creature->GetInstanceScript();
    }

    InstanceScript* instance;
    EventMap events;

    void Reset() override
    {
        events.Reset();
    }

    void EnterCombat(Unit* /*who*/) override
    {
        if (urand(0, 1))
            events.ScheduleEvent(EVENT_ANTI_MAGIC_PRISON, urand(3000, 5000));
        else
            events.ScheduleEvent(EVENT_SHADOW_BOLT_VOLLEY, urand(3000, 5000));
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_ANTI_MAGIC_PRISON:
                if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                    DoCast(target, SPELL_ANTI_MAGIC_PRISON, false);

                events.ScheduleEvent(EVENT_ANTI_MAGIC_PRISON, urand(31000, 33000));
                break;
            case EVENT_SHADOW_BOLT_VOLLEY:
                DoCast(SPELL_SHADOW_BOLT_VOLLEY);
                events.ScheduleEvent(EVENT_SHADOW_BOLT_VOLLEY, urand(9000, 13000));
                break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

void AddSC_boss_setesh()
{
    RegisterCreatureAI(boss_setesh);
    RegisterCreatureAI(npc_setesh_chaos_portal);
    RegisterCreatureAI(npc_setesh_void_sentinel);
    RegisterCreatureAI(npc_setesh_void_seeker);
}
