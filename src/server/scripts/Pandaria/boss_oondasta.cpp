
//World boss

enum eSpells
{
    SPELL_CRUSH           = 137504,
    SPELL_FRILL_BLAST     = 137505,
    SPELL_GROWING_FURY    = 137502,
    SPELL_PIERCING_ROAR   = 137457,
};

enum eEvents
{
    EVENT_CRUSH           = 1,
    EVENT_FRILL_BLAST     = 2,
    EVENT_GROWING_FURY    = 3,
    EVENT_PIERCING_ROAR   = 4,
    EVENT_RE_ATTACK       = 5,
};

//69161
class boss_oondasta : public CreatureScript
{
public:
    boss_oondasta() : CreatureScript("boss_oondasta") { }

    struct boss_oondastaAI : public CreatureAI
    {
        boss_oondastaAI(Creature* creature) : CreatureAI(creature){}

        EventMap events;

        void Reset() override
        {
            events.Reset();
            me->SetReactState(REACT_DEFENSIVE);
        }

        void EnterCombat(Unit* unit) override
        {
            DoZoneInCombat(me, 75.0f);
            events.RescheduleEvent(EVENT_CRUSH, 60000);
            events.RescheduleEvent(EVENT_FRILL_BLAST, urand(25000, 60000));
            events.RescheduleEvent(EVENT_PIERCING_ROAR, urand(25000, 30000));
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim() || me->HasUnitState(UNIT_STATE_CASTING))
                return;

            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_CRUSH:
                    if (me->getVictim())
                        DoCast(me->getVictim(), SPELL_CRUSH);
                    events.RescheduleEvent(EVENT_CRUSH, 26000);
                    break;
                case EVENT_FRILL_BLAST:
                    {
                        me->AttackStop();
                        me->SetReactState(REACT_PASSIVE);
                        DoCastAOE(SPELL_FRILL_BLAST);
                        events.DelayEvents(3000);
                        events.RescheduleEvent(EVENT_RE_ATTACK, 2500);
                        events.RescheduleEvent(EVENT_FRILL_BLAST, urand(25000, 60000));
                    }
                    break;
                case EVENT_PIERCING_ROAR:
                    DoCastAOE(SPELL_PIERCING_ROAR);
                    events.RescheduleEvent(EVENT_PIERCING_ROAR, urand(25000, 30000));
                    break;
                case EVENT_RE_ATTACK:
                    me->SetReactState(REACT_AGGRESSIVE);
                    DoZoneInCombat(me, 75.0f);
                    break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_oondastaAI(creature);
    }
};

void AddSC_boss_oondasta()
{
    new boss_oondasta();
}
