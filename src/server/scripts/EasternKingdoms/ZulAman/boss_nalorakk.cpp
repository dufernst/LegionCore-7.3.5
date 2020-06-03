#include "zulaman.h"

enum ScriptTexts
{
    SAY_AGGRO   = 4, 
    SAY_BEAR    = 6,
    SAY_TROLL   = 7,
    SAY_SURGE   = 5,
    SAY_KILL    = 8,
    SAY_DEATH   = 9,
};

enum Spells
{
    SPELL_BEARFORM          = 42377,
    SPELL_BRUTAL_STRIKE     = 42384,
    SPELL_SURGE             = 42402,
    SPELL_LACERATING_SLASH  = 42395,
    SPELL_DEAFENING_ROAR    = 49721,
};

enum Events
{
    EVENT_BEARFORM          = 1,
    EVENT_BRUTAL_STRIKE     = 2,
    EVENT_SURGE             = 3,
    EVENT_LACERATING_SLASH  = 4,
    EVENT_DEAFENING_ROAR    = 5,
    EVENT_TROLLFORM         = 6,
};

class boss_nalorakk : public CreatureScript
{
    public:
        boss_nalorakk() : CreatureScript("boss_nalorakk") {}
        
        CreatureAI* GetAI(Creature* pCreature) const
        {
            return GetInstanceAI<boss_nalorakkAI>(pCreature);
        }

        struct boss_nalorakkAI : public BossAI
        {
            boss_nalorakkAI(Creature* pCreature) : BossAI(pCreature, DATA_NALORAKK)
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

            void Reset()
            {
                _Reset();
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_ATTACKABLE_1 | UNIT_FLAG_IMMUNE_TO_PC);
            }

            void EnterCombat(Unit* who)
            {
                Talk(SAY_AGGRO);
                events.RescheduleEvent(EVENT_BEARFORM, 30000);
                events.RescheduleEvent(EVENT_BRUTAL_STRIKE, 6000);
                DoZoneInCombat();
                instance->SetBossState(DATA_NALORAKK, IN_PROGRESS);
            }

            void JustDied(Unit* /*killer*/)
            {
                _JustDied();
                Talk(SAY_DEATH);
            }

            void KilledUnit(Unit* /*victim*/)
            {
                Talk(SAY_KILL);
            }

            void UpdateAI(uint32 diff)
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
                        case EVENT_SURGE:
                            Talk(SAY_SURGE);
                            if (Unit* pTarget = SelectTarget(SELECT_TARGET_FARTHEST, 0, 0.0f, true))
                                DoCast(pTarget, SPELL_SURGE);
                            break;
                        case EVENT_BRUTAL_STRIKE:
                            DoCastVictim(SPELL_BRUTAL_STRIKE);
                            events.RescheduleEvent(EVENT_SURGE, 2000);
                            events.RescheduleEvent(EVENT_BRUTAL_STRIKE, 8000);
                            break;
                        case EVENT_BEARFORM:
                            Talk(SAY_BEAR);
                            events.CancelEvent(EVENT_SURGE);
                            events.CancelEvent(EVENT_BRUTAL_STRIKE);
                            DoCast(me, SPELL_BEARFORM);
                            events.RescheduleEvent(EVENT_LACERATING_SLASH, 6000);
                            events.RescheduleEvent(EVENT_DEAFENING_ROAR, urand(3000, 8000));
                            events.RescheduleEvent(EVENT_TROLLFORM, 30000);
                            break;
                        case EVENT_LACERATING_SLASH:
                            DoCastVictim(SPELL_LACERATING_SLASH);
                            events.RescheduleEvent(EVENT_LACERATING_SLASH, 20000);
                            break;
                        case EVENT_DEAFENING_ROAR:
                            DoCastAOE(SPELL_DEAFENING_ROAR);
                            events.RescheduleEvent(EVENT_DEAFENING_ROAR, 8000);
                            break;
                        case EVENT_TROLLFORM:
                            Talk(SAY_TROLL);
                            events.CancelEvent(EVENT_LACERATING_SLASH);
                            events.CancelEvent(EVENT_DEAFENING_ROAR);
                            events.RescheduleEvent(EVENT_BEARFORM, 30000);
                            events.RescheduleEvent(EVENT_BRUTAL_STRIKE, 6000);
                            break;
                     }
                }

                DoMeleeAttackIfReady();
            }
        };     
};

void AddSC_boss_nalorakk()
{
    new boss_nalorakk();
}