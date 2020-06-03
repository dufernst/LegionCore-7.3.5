#include "ScriptedEscortAI.h"

enum eBonobosSpells
{
    SPELL_GOING_BANANAS     = 125363,
    SPELL_BANANARANG        = 125311,
    SPELL_TOSS_FILTH        = 125365,
};

enum eBonobosEvents
{
    EVENT_GOING_BANANAS         = 1,
    EVENT_BANANARANG            = 2,
    EVENT_TOSS_FILTH            = 3,
};

class mob_bonobos : public CreatureScript
{
    public:
        mob_bonobos() : CreatureScript("mob_bonobos") 
        { 
        }

        CreatureAI* GetAI(Creature* creature) const
        {
            return new mob_bonobosAI(creature);
        }

        struct mob_bonobosAI : public ScriptedAI
        {
            mob_bonobosAI(Creature* creature) : ScriptedAI(creature)
            {
            }

            EventMap events;

            void Reset()
            {
                events.Reset();
                
                events.RescheduleEvent(EVENT_GOING_BANANAS,        12000);
                events.RescheduleEvent(EVENT_BANANARANG,             8000);
                events.RescheduleEvent(EVENT_TOSS_FILTH,            15000);
            }

            void JustDied(Unit* /*killer*/)
            {
            }

            void UpdateAI(uint32 diff)
            {
                if (!UpdateVictim())
                    return;
                
                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                events.Update(diff);
                

                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_GOING_BANANAS:
                            if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                                me->CastSpell(target, SPELL_GOING_BANANAS, false);
                            events.RescheduleEvent(EVENT_GOING_BANANAS,      10000);
                            break;
                        case EVENT_BANANARANG:
                            if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                                me->CastSpell(target, SPELL_BANANARANG, false);
                            events.RescheduleEvent(EVENT_BANANARANG, 20000);
                            break;
                        case EVENT_TOSS_FILTH:
                            if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                                me->CastSpell(target, SPELL_TOSS_FILTH, false);
                            events.RescheduleEvent(EVENT_TOSS_FILTH, 15000);
                            break;
                        default:
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }
        };
};

void AddSC_valley_of_the_four_winds()
{
    new mob_bonobos();
}