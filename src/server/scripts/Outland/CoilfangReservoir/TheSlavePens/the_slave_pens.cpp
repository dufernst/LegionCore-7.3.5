
#include "the_slave_pens.h"
#include "LFGMgr.h"
#include "Group.h"

// BOSS: MENNU THE BETRAYER ID: 17941

enum SayMennu
{
    SAY_AGGRO                       = 0,
    SAY_SLAY                        = 1,
    SAY_DEATH                       = 2
};

enum SpellsMennu
{
    SPELL_TAINTED_STONESKIN_TOTEM   = 31985,
    SPELL_TAINTED_EARTHGRAB_TOTEM   = 31981,
    SPELL_CORRUPTED_NOVA_TOTEM      = 31991,
    SPELL_MENNUS_HEALING_WARD       = 34980,
    SPELL_LIGHTNING_BOLT            = 35010
};

enum EventsMennu
{
    EVENT_TAINTED_STONESKIN_TOTEM   = 1,
    EVENT_TAINTED_EARTHGRAB_TOTEM   = 2,
    EVENT_CORRUPTED_NOVA_TOTEM      = 3,
    EVENT_MENNUS_HEALING_WARD       = 4,
    EVENT_LIGHTNING_BOLT            = 5
};

class boss_mennu_the_betrayer : public CreatureScript
{
public:
    boss_mennu_the_betrayer() : CreatureScript("boss_mennu_the_betrayer") { }

    struct boss_mennu_the_betrayerAI : public BossAI
    {
        boss_mennu_the_betrayerAI(Creature* creature) : BossAI(creature, DATA_MENNU) { }

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
            events.RescheduleEvent(EVENT_TAINTED_STONESKIN_TOTEM, 30000);
            events.RescheduleEvent(EVENT_TAINTED_EARTHGRAB_TOTEM, 20000);
            events.RescheduleEvent(EVENT_CORRUPTED_NOVA_TOTEM, 60000);
            events.RescheduleEvent(EVENT_MENNUS_HEALING_WARD, urand(14000, 25000));
            events.RescheduleEvent(EVENT_LIGHTNING_BOLT, urand(14000, 19000));
            Talk(SAY_AGGRO);
        }

        void KilledUnit(Unit* /*victim*/) override
        {
            Talk(SAY_SLAY);
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
                case EVENT_TAINTED_STONESKIN_TOTEM:
                    if (HealthBelowPct(100))
                        DoCast(me, SPELL_TAINTED_STONESKIN_TOTEM);
                    events.RescheduleEvent(EVENT_TAINTED_STONESKIN_TOTEM, 30000);
                    break;
                case EVENT_TAINTED_EARTHGRAB_TOTEM:
                    DoCast(me, SPELL_TAINTED_EARTHGRAB_TOTEM);
                    break;
                case EVENT_CORRUPTED_NOVA_TOTEM:
                    DoCast(me, SPELL_CORRUPTED_NOVA_TOTEM);
                    break;
                case EVENT_MENNUS_HEALING_WARD:
                    DoCast(me, SPELL_MENNUS_HEALING_WARD);
                    events.RescheduleEvent(EVENT_MENNUS_HEALING_WARD, urand(14000, 25000));
                    break;
                case EVENT_LIGHTNING_BOLT:
                    DoCast(SPELL_LIGHTNING_BOLT);
                    events.RescheduleEvent(EVENT_LIGHTNING_BOLT, urand(14000, 25000));
                    break;
                default:
                    break;
                }

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_mennu_the_betrayerAI(creature);
    }
};

// BOSS: ROKMAR THE CRACKER ID: 17991

enum SpellsRokm
{
    SPELL_GRIEVOUS_WOUND            = 31956,
    SPELL_ENSNARING_MOSS            = 31948,
    SPELL_WATER_SPIT                = 35008,
    SPELL_FRENZY                    = 34970
};

enum EventsRokm
{
    EVENT_GRIEVOUS_WOUND            = 1,
    EVENT_ENSNARING_MOSS            = 2,
    EVENT_WATER_SPIT                = 3
};

class boss_rokmar_the_crackler : public CreatureScript
{
public:
    boss_rokmar_the_crackler() : CreatureScript("boss_rokmar_the_crackler") { }

    struct boss_rokmar_the_cracklerAI : public BossAI
    {
        boss_rokmar_the_cracklerAI(Creature* creature) : BossAI(creature, DATA_ROKMAR)
        {
            Initialize();
        }

        void Initialize()
        {
            rokmarFrenzy = false;
        }

        void Reset() override
        {
            _Reset();
            Initialize();
        }

        void JustDied(Unit* /*killer*/) override
        {
            _JustDied();
        }

        void EnterCombat(Unit* /*who*/) override
        {
            _EnterCombat();
            events.RescheduleEvent(EVENT_GRIEVOUS_WOUND, 10000);
            events.RescheduleEvent(EVENT_ENSNARING_MOSS, 20000);
            events.RescheduleEvent(EVENT_WATER_SPIT, 14000);
        }

        void KilledUnit(Unit* /*victim*/) override { }

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
                case EVENT_GRIEVOUS_WOUND:
                    DoCast(SPELL_GRIEVOUS_WOUND);
                    events.RescheduleEvent(EVENT_GRIEVOUS_WOUND, urand(20000, 30000));
                    break;
                case EVENT_ENSNARING_MOSS:
                    DoCast(SPELL_ENSNARING_MOSS);
                    events.RescheduleEvent(EVENT_ENSNARING_MOSS, urand(20000, 30000));
                    break;
                case EVENT_WATER_SPIT:
                    DoCast(SPELL_WATER_SPIT);
                    events.RescheduleEvent(EVENT_WATER_SPIT, urand(14000, 18000));
                    break;
                default:
                    break;
                }

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;
            }

            if (HealthBelowPct(10) && !rokmarFrenzy)
            {
                DoCast(me, SPELL_FRENZY);
                rokmarFrenzy = true;
            }

            DoMeleeAttackIfReady();
        }

    private:
        bool   rokmarFrenzy;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_rokmar_the_cracklerAI(creature);
    }
};

// BOSS: QUAGMIRRAN ID: 17942

enum SpellsQuag
{
    SPELL_ACID_SPRAY                = 38153,
    SPELL_CLEAVE                    = 40504,
    SPELL_UPPERCUT                  = 32055,
    SPELL_POISON_BOLT_VOLLEY        = 34780
};

enum EventsQuag
{
    EVENT_ACID_SPRAY                = 1,
    EVENT_CLEAVE                    = 2,
    EVENT_UPPERCUT                  = 3,
    EVENT_POISON_BOLT_VOLLEY        = 4
};

class boss_quagmirran : public CreatureScript
{
public:
    boss_quagmirran() : CreatureScript("boss_quagmirran") { }

    struct boss_quagmirranAI : public BossAI
    {
        boss_quagmirranAI(Creature* creature) : BossAI(creature, DATA_QUAGMIRRAN) { }

        void Reset() override
        {
            _Reset();
        }

        void JustDied(Unit* /*killer*/) override
        {
            _JustDied();
            if (instance)
            {
                Map::PlayerList const& players = me->GetMap()->GetPlayers();
                if (!players.isEmpty())
                {
                    Player* pPlayer = players.begin()->getSource();
                    if (pPlayer && pPlayer->GetGroup())
                        if (sLFGMgr->GetQueueId(744))
                            sLFGMgr->FinishDungeon(pPlayer->GetGroup()->GetGUID(), 744);
                }
            }
        }

        void EnterCombat(Unit* /*who*/) override
        {
            _EnterCombat();
            events.RescheduleEvent(EVENT_ACID_SPRAY, 25000);
            events.RescheduleEvent(EVENT_CLEAVE, 9000);
            events.RescheduleEvent(EVENT_UPPERCUT, 20000);
            events.RescheduleEvent(EVENT_POISON_BOLT_VOLLEY, 31000);
        }

        void KilledUnit(Unit* /*victim*/) override { }

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
                case EVENT_ACID_SPRAY:
                    DoCast(SPELL_ACID_SPRAY);
                    events.RescheduleEvent(EVENT_ACID_SPRAY, urand(20000, 25000));
                    break;
                case EVENT_CLEAVE:
                    DoCast(SPELL_CLEAVE);
                    events.RescheduleEvent(EVENT_CLEAVE, urand(18000, 34000));
                    break;
                case EVENT_UPPERCUT:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 10.0f, true))
                        DoCast(target, SPELL_UPPERCUT);
                    events.RescheduleEvent(EVENT_UPPERCUT, 22000);
                    break;
                case EVENT_POISON_BOLT_VOLLEY:
                    DoCast(me, SPELL_POISON_BOLT_VOLLEY);
                    events.RescheduleEvent(EVENT_POISON_BOLT_VOLLEY, 24000);
                    break;
                default:
                    break;
                }

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_quagmirranAI(creature);
    }
};

void AddSC_the_slave_pens()
{
    new boss_mennu_the_betrayer();
    new boss_rokmar_the_crackler();
    new boss_quagmirran();
}