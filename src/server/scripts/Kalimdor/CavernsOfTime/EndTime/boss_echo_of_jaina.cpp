#include "end_time.h"

enum Yells
{
    SAY_AGGRO   = 0,
    SAY_DEATH   = 1,
    SAY_INTRO   = 2,
    SAY_KILL    = 3,   
    SAY_SPELL   = 4
};

enum Spells
{
    SPELL_BLINK                 = 101812,
    SPELL_FLARECORE_MISSILE     = 101927,
    SPELL_UNSTABLE_FLARE        = 101980,
    SPELL_TIME_EXPIRE_FLARE     = 101587, 
    SPELL_CHECK_PLAYER_DIST     = 101588,

    SPELL_FROSTBOLT_VOLLEY      = 101810,
    SPELL_PYROBLAST             = 101809,

    SPELL_FROST_BLADES_SUMMON   = 101339
};

enum Events
{
    EVENT_FLARECORE         = 1, 
    EVENT_BLINK             = 2, 
    EVENT_FROSTBOLT_VOLLEY  = 3, 
    EVENT_PYROBLAST         = 4, 
    EVENT_FROST_BLADES      = 5, 
    EVENT_CHECK_PLAYER      = 6,
    EVENT_EXPLODE           = 7
};

enum Creatures
{
    NPC_FLARECORE       = 54446,
    NPC_FROSTBLADES     = 54494,
    NPC_BLINK_TARGET    = 54542,
    NPC_ARCANE_CIRCLE   = 54639,
    NPC_JAINA           = 54641
};

enum Others
{
    ACTION_FRAGMENTS = 1
};

uint32 FragmentsCount = 1;

static const Position jainaPos = {3004.780029f, 515.729004f, 21.55f, 3.12f};

struct boss_echo_of_jaina : public BossAI
{
    explicit boss_echo_of_jaina(Creature* creature) : BossAI(creature, DATA_ECHO_OF_JAINA)
    {
        me->SetReactState(REACT_PASSIVE);
        me->setActive(true);
        me->SetVisible(false);
        bIntro = false;
        uiVolleyCount = 0;
        uiFragments = 0;
    }

    uint32 uiVolleyCount;
    uint8 uiFragments;
    bool bIntro;

    void Reset() override
    {
        _Reset();
        uiVolleyCount = 0;

        if (instance->GetData(DATA_JAINA_EVENT) == DONE)
        {
            uiFragments = MAX_FRAGMENTS_COUNT;
            me->SetVisible(true);
            me->SetReactState(REACT_AGGRESSIVE);
        }
        else
            me->SummonCreature(NPC_ARCANE_CIRCLE, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0.0f);
    }

    void DoAction(int32 const action) override
    {
        if (action == ACTION_FRAGMENTS)
        {
            if (uiFragments >= MAX_FRAGMENTS_COUNT)
                return;

            if (!bIntro)
            {
                bIntro = true;
                Talk(SAY_INTRO);
            }
            uiFragments++;

            if (uiFragments >= MAX_FRAGMENTS_COUNT)
            {
                instance->SetData(DATA_JAINA_EVENT, DONE);
                me->SetVisible(true);
                me->SetReactState(REACT_AGGRESSIVE);
                summons.DespawnEntry(NPC_ARCANE_CIRCLE);
            }

            instance->SetData(DATA_FRAGMENTS, uiFragments);
            instance->DoUpdateWorldState(WorldStates::WORLDSTATE_SHOW_FRAGMENTS, 1);
            instance->DoUpdateWorldState(WorldStates::WORLDSTATE_FRAGMENTS_COLLECTED, uiFragments);
        }
    }

    void JustDied(Unit* /*killer*/) override
    {
        _JustDied();
        Talk(SAY_DEATH);

        // Quest
        instance->instance->ApplyOnEveryPlayer([&](Player* player)
        {
            if (me->GetDistance2d(player) <= 50.0f && player->GetQuestStatus(30097) == QUEST_STATUS_INCOMPLETE)
                DoCast(player, SPELL_ARCHIVED_JAINA, true);
        });
    }

    void KilledUnit(Unit* /*victim*/) override
    {
        Talk(SAY_KILL);
    }

    void JustSummoned(Creature* summon) override
    {
        BossAI::JustSummoned(summon);

        switch (summon->GetEntry())
        {
        case NPC_FROSTBLADES:
        {
            summon->SetReactState(REACT_PASSIVE);
            float x, y, z;
            summon->GetClosePoint(x, y, z, me->GetObjectSize() / 3, 100.0f);
            summon->GetMotionMaster()->MovePoint(1, x, y, z);
            summon->DespawnOrUnsummon(10000);
            break;
        }
        default:
            break;
        }
    }

    void EnterCombat(Unit* /*who*/) override
    {
        _EnterCombat();
        Talk(SAY_AGGRO);

        events.ScheduleEvent(EVENT_BLINK, 30000);
        events.ScheduleEvent(EVENT_PYROBLAST, 1000);
        events.ScheduleEvent(EVENT_FLARECORE, urand(10000, 20000));

        instance->SetBossState(DATA_ECHO_OF_JAINA, IN_PROGRESS);
        DoZoneInCombat();
    }

    void AttackStart(Unit* who) override
    {
        if (who)
            me->Attack(who, false);
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
            case EVENT_PYROBLAST:
                if (auto victim = me->getVictim())
                    DoCast(victim, SPELL_PYROBLAST, false);

                events.ScheduleEvent(EVENT_PYROBLAST, urand(3500, 4500));
                break;
            case EVENT_FLARECORE:
                if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                    DoCast(target, SPELL_FLARECORE_MISSILE, false);

                events.ScheduleEvent(EVENT_FLARECORE, urand(19000, 21000));
                break;
            case EVENT_BLINK:
            {
                std::list<Creature*> creatures;
                me->GetCreatureListWithEntryInGrid(creatures, NPC_BLINK_TARGET, 40.0f);
                if (!creatures.empty())
                {
                    Trinity::Containers::RandomResizeList(creatures, 1);
                    DoCast(me, SPELL_BLINK, true);

                    if (auto target = creatures.front())
                        me->NearTeleportTo(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), 0.0f, true);
                }
                me->UpdateObjectVisibility();
                events.CancelEvent(EVENT_PYROBLAST);
                events.CancelEvent(EVENT_FLARECORE);
                events.ScheduleEvent(EVENT_FROST_BLADES, 1500);
                events.ScheduleEvent(EVENT_FROSTBOLT_VOLLEY, 2000);
                events.ScheduleEvent(EVENT_BLINK, 31500);
                break;
            }
            case EVENT_FROSTBOLT_VOLLEY:
                if (uiVolleyCount < 3)
                {
                    DoCast(SPELL_FROSTBOLT_VOLLEY);
                    events.ScheduleEvent(EVENT_FROSTBOLT_VOLLEY, 2200);
                    uiVolleyCount++;
                }
                else
                {
                    events.CancelEvent(EVENT_FROSTBOLT_VOLLEY);
                    uiVolleyCount = 0;
                    events.ScheduleEvent(EVENT_PYROBLAST, urand(3000, 3500));
                    events.ScheduleEvent(EVENT_FLARECORE, urand(7500, 8500));
                }
                break;
            case EVENT_FROST_BLADES:
                Talk(SAY_SPELL);
                me->SummonCreature(NPC_FROSTBLADES, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation() - (M_PI / 4.0f), TEMPSUMMON_TIMED_DESPAWN, 30000);
                me->SummonCreature(NPC_FROSTBLADES, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 30000);
                me->SummonCreature(NPC_FROSTBLADES, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation() + (M_PI / 4.0f), TEMPSUMMON_TIMED_DESPAWN, 30000);
                break;
            }
        }
    }
};

struct npc_echo_of_jaina_blink_target : public ScriptedAI
{
    explicit npc_echo_of_jaina_blink_target(Creature* creature) : ScriptedAI(creature)
    {
        me->SetReactState(REACT_PASSIVE);
    }
};

struct npc_echo_of_jaina_flarecore : public ScriptedAI
{
    explicit npc_echo_of_jaina_flarecore(Creature* creature) : ScriptedAI(creature)
    {
        me->SetReactState(REACT_PASSIVE);
    }

    EventMap events;

    void Reset() override
    {
        if (!me->HasAura(SPELL_CHECK_PLAYER_DIST))
            me->AddAura(SPELL_CHECK_PLAYER_DIST, me);
    }

    void EnterCombat(Unit* /*who*/) override
    {
        events.ScheduleEvent(EVENT_CHECK_PLAYER, 500);
        events.ScheduleEvent(EVENT_EXPLODE, 10000);
    }

    void UpdateAI(uint32 diff) override
    {
        events.Update(diff);

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_CHECK_PLAYER:
                if (me->SelectNearestPlayer(3.0f))
                {
                    DoCast(SPELL_UNSTABLE_FLARE);
                    me->DespawnOrUnsummon(100);
                }
                else
                    events.ScheduleEvent(EVENT_CHECK_PLAYER, 500);
                break;
            case EVENT_EXPLODE:
                DoCast(SPELL_TIME_EXPIRE_FLARE);
                me->DespawnOrUnsummon(100);
                break;
            }
        }
    }
};

class go_echo_of_jaina_jaina_staff_fragment : public GameObjectScript
{
    public:
        go_echo_of_jaina_jaina_staff_fragment() : GameObjectScript("go_echo_of_jaina_jaina_staff_fragment") {}

        bool OnGossipHello(Player* /*player*/, GameObject* go)
        {
            InstanceScript* instance = go->GetInstanceScript();
            if (!instance)
                return true;

            if (auto jaina = ObjectAccessor::GetCreature(*go, instance->GetGuidData(DATA_ECHO_OF_JAINA)))
                jaina->AI()->DoAction(ACTION_FRAGMENTS);
 
            go->SetFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_IN_USE);
            go->SetGoState(GO_STATE_ACTIVE);
            return true;
        }
};

void AddSC_boss_echo_of_jaina()
{
    RegisterCreatureAI(boss_echo_of_jaina);
    RegisterCreatureAI(npc_echo_of_jaina_flarecore);
    RegisterCreatureAI(npc_echo_of_jaina_blink_target);
    new go_echo_of_jaina_jaina_staff_fragment();
}
