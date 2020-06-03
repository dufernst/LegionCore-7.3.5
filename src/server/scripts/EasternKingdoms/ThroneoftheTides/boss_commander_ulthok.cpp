#include "throne_of_the_tides.h"

enum Spells
{
    SPELL_DARK_FISSURE          = 76047,
    SPELL_SQUEEZE               = 76026,
    SPELL_SQUEEZE_VEHICLE       = 76028,
    SPELL_ENRAGE                = 76100,
    SPELL_CURSE_OF_FATIGUE      = 76094,
    SPELL_DARK_FISSURE_AURA     = 76066,
    SPELL_DARK_FISSURE_DMG      = 76085,
    SPELL_ULTHOK_INTRO          = 82960
};

enum Events
{
    EVENT_DARK_FISSURE      = 1,
    EVENT_SQUEEZE           = 2,
    EVENT_CURSE_OF_FATIGUE  = 3,
    EVENT_ENRAGE            = 4
};

enum Actions
{
    ACTION_COMMANDER_ULTHOK_START_EVENT = 2
};

enum Adds
{
    NPC_DARK_FISSURE = 40784
};

struct boss_commander_ulthok : public BossAI
{
    explicit boss_commander_ulthok(Creature* creature) : BossAI(creature, DATA_COMMANDER_ULTHOK) {}

    void Reset() override
    {
        _Reset();
        me->AddAura(76017, me);
    }

    void DoAction(int32 const action) override
    {
        if (action == ACTION_COMMANDER_ULTHOK_START_EVENT)
        {
            me->SetPhaseMask(PHASEMASK_NORMAL, true);
            DoCast(SPELL_ULTHOK_INTRO);

            if (auto corales = ObjectAccessor::GetGameObject(*me, instance->GetGuidData(DATA_CORALES)))
            {
                corales->SetGoState(GO_STATE_ACTIVE_ALTERNATIVE);
                corales->SetPhaseMask(2, true);
            }
        }
    }

    void EnterCombat(Unit* /*who*/) override
    {
        Talk(0);
        Talk(1);
        _EnterCombat();
        events.RescheduleEvent(EVENT_DARK_FISSURE, urand(5000, 8000));
        events.RescheduleEvent(EVENT_ENRAGE, urand(20000, 25000));
        events.RescheduleEvent(EVENT_CURSE_OF_FATIGUE, urand(9000, 15000));
        events.RescheduleEvent(EVENT_SQUEEZE, urand(14000, 20000));
        me->RemoveAura(76017);
    }

    void JustDied(Unit* /*killer*/) override
    {
        Talk(2);
        Talk(3);
        _JustDied();
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
            case EVENT_DARK_FISSURE:
                DoCast(SPELL_DARK_FISSURE);
                events.RescheduleEvent(EVENT_DARK_FISSURE, urand(20000, 22000));
                break;
            case EVENT_ENRAGE:
                DoCast(SPELL_ENRAGE);
                events.RescheduleEvent(EVENT_ENRAGE, urand(20000, 25000));
                break;
            case EVENT_SQUEEZE:
                if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                    DoCast(target, SPELL_SQUEEZE, false);

                events.RescheduleEvent(EVENT_SQUEEZE, urand(29000, 31000));
                break;
            case EVENT_CURSE_OF_FATIGUE:
                if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                    DoCast(target, SPELL_CURSE_OF_FATIGUE, false);

                events.RescheduleEvent(EVENT_CURSE_OF_FATIGUE, urand(13000, 15000));
                break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

struct npc_ulthok_dark_fissure : public ScriptedAI
{
    explicit npc_ulthok_dark_fissure(Creature* creature) : ScriptedAI(creature)
    {
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
        me->SetReactState(REACT_PASSIVE);
    }

    void Reset() override
    {
        DoCast(me, SPELL_DARK_FISSURE_AURA, true);
    }
};

class at_tott_commander_ulthok : public AreaTriggerScript
{
    public:
        at_tott_commander_ulthok() : AreaTriggerScript("at_tott_commander_ulthok") {}

        bool OnTrigger(Player* player, const AreaTriggerEntry* /*at*/, bool /*enter*/)
        {
            if (InstanceScript* instance = player->GetInstanceScript())
            {
                if (instance->GetData(DATA_COMMANDER_ULTHOK_EVENT) != DONE && instance->GetBossState(DATA_LADY_NAZJAR) == DONE)
                {
                    instance->SetData(DATA_COMMANDER_ULTHOK_EVENT, DONE);

                    if (auto ulthok = ObjectAccessor::GetCreature(*player, instance->GetGuidData(DATA_COMMANDER_ULTHOK)))
                        ulthok->AI()->DoAction(ACTION_COMMANDER_ULTHOK_START_EVENT);
                }
            }
            return true;
        }
};

void AddSC_boss_commander_ulthok()
{
    RegisterCreatureAI(boss_commander_ulthok);
    RegisterCreatureAI(npc_ulthok_dark_fissure);
    new at_tott_commander_ulthok();
}