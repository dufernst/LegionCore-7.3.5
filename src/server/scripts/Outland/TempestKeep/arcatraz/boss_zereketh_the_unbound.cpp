/*

    Arcatraz: Zereketh the unbound

*/

#include "arcatraz.h"

enum Say
{
    SAY_AGGRO                   = 0,
    SAY_KILL                    = 1,
    SAY_NOVA                    = 2,
    SAY_DEATH                   = 3
};

enum Spells
{
    SPELL_VOID_ZONE             = 36119,
    SPELL_SHADOW_NOVA           = 36127,
    SPELL_SEED_OF_CORRUPTION    = 36123
};

enum Events
{
    EVENT_VOID_ZONE             = 1,
    EVENT_SHADOW_NOVA           = 2,
    EVENT_SEED_OF_CORRUPTION    = 3
};


class boss_zereketh_the_unbound : public CreatureScript
{
public:
    boss_zereketh_the_unbound() : CreatureScript("boss_zereketh_the_unbound") { }

    struct boss_zereketh_the_unboundAI : public BossAI
    {
        boss_zereketh_the_unboundAI(Creature* creature) : BossAI(creature, DATA_ZEREKETH) { }

        void Reset() override
        {
            _Reset();
        }

        void JustDied(Unit* /*killer*/) override
        {
            _JustDied();
            Talk(SAY_DEATH);
        }

        void EnterCombat(Unit* /*who*/) override
        {
            _EnterCombat();
            events.ScheduleEvent(EVENT_VOID_ZONE, urand(8000, 10000));
            events.ScheduleEvent(EVENT_SHADOW_NOVA, urand(10000, 12000));
            events.ScheduleEvent(EVENT_SEED_OF_CORRUPTION, urand(12000, 20000));
            Talk(SAY_AGGRO);
        }

        void KilledUnit(Unit* /*victim*/) override
        {
            Talk(SAY_KILL);
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
                case EVENT_VOID_ZONE:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 100, true))
                        DoCast(target, SPELL_VOID_ZONE);
                    events.ScheduleEvent(EVENT_VOID_ZONE, urand(8000, 10000));
                    break;
                case EVENT_SHADOW_NOVA:
                    DoCastVictim(SPELL_SHADOW_NOVA, true);
                    Talk(SAY_NOVA);
                    events.ScheduleEvent(EVENT_SHADOW_NOVA, urand(10000, 12000));
                    break;
                case EVENT_SEED_OF_CORRUPTION:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 100, true))
                        DoCast(target, SPELL_SEED_OF_CORRUPTION);
                    events.ScheduleEvent(EVENT_SEED_OF_CORRUPTION, urand(12000, 20000));
                    break;
                default:
                    break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_zereketh_the_unboundAI(creature);
    }
};

void AddSC_boss_zereketh_the_unbound()
{
    new boss_zereketh_the_unbound();
}

