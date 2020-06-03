/*

    Arcatraz: Dalliah the doomslayer

*/

#include "arcatraz.h"
#include "InstanceScript.h"
#include "ObjectAccessor.h"
#include "ScriptedCreature.h"

enum Say
{
    // Dalliah the Doomsayer
    SAY_AGGRO                   = 1,
    SAY_SLAY                    = 2,
    SAY_WHIRLWIND               = 3,
    SAY_HEAL                    = 4,
    SAY_DEATH                   = 5,
    SAY_SOCCOTHRATES_DEATH      = 7,

    // Wrath-Scryer Soccothrates
    SAY_AGGRO_DALLIAH_FIRST     = 0,
    SAY_DALLIAH_25_PERCENT      = 5
};

enum Spells
{
    SPELL_GIFT_OF_THE_DOOMSAYER = 36173,
    SPELL_WHIRLWIND             = 36142,
    SPELL_HEAL                  = 36144,
    SPELL_SHADOW_WAVE           = 39016
};

enum Events
{
    EVENT_GIFT_OF_THE_DOOMSAYER = 1,
    EVENT_WHIRLWIND             = 2,
    EVENT_HEAL                  = 3,
    EVENT_SHADOW_WAVE           = 4,
    EVENT_ME_FIRST              = 5,
    EVENT_SOCCOTHRATES_DEATH    = 6
};

// 20885
class boss_dalliah_the_doomsayer : public CreatureScript
{
public:
    boss_dalliah_the_doomsayer() : CreatureScript("boss_dalliah_the_doomsayer") { }

    struct boss_dalliah_the_doomsayerAI : public BossAI
    {
        boss_dalliah_the_doomsayerAI(Creature* creature) : BossAI(creature, DATA_DALLIAH)
        {
            soccothratesTaunt = false;
            soccothratesDeath = false;
        }

        void Reset() override
        {
            _Reset();
            soccothratesTaunt = false;
            soccothratesDeath = false;
        }

        void JustDied(Unit* /*killer*/) override
        {
            _JustDied();
            Talk(SAY_DEATH);

            if (Creature* soccothrates = me->FindNearestCreature(NPC_SOCCOTHRATES, 100.0f, true))
                if (!soccothrates->isInCombat())
                    soccothrates->AI()->SetData(1, 1);
        }

        void EnterCombat(Unit* /*who*/) override
        {
            _EnterCombat();
            events.ScheduleEvent(EVENT_GIFT_OF_THE_DOOMSAYER, urand(1000, 4000));
            events.ScheduleEvent(EVENT_WHIRLWIND, urand(7000, 9000));
            if (IsHeroic())
                events.ScheduleEvent(EVENT_SHADOW_WAVE, urand(11000, 16000));
            events.ScheduleEvent(EVENT_ME_FIRST, 6000);
            Talk(SAY_AGGRO);
        }

        void KilledUnit(Unit* /*victim*/) override
        {
            Talk(SAY_SLAY);
        }

        void SetData(uint32 /*type*/, uint32 data) override
        {
            switch (data)
            {
            case 1:
                events.ScheduleEvent(EVENT_SOCCOTHRATES_DEATH, 6000);
                soccothratesDeath = true;
                break;
            default:
                break;
            }
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
            {
                if (soccothratesDeath)
                {
                    events.Update(diff);

                    while (uint32 eventId = events.ExecuteEvent())
                    {
                        switch (eventId)
                        {
                        case EVENT_SOCCOTHRATES_DEATH:
                            Talk(SAY_SOCCOTHRATES_DEATH);
                            break;
                        default:
                            break;
                        }
                    }
                }

                return;
            }

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_GIFT_OF_THE_DOOMSAYER:
                    DoCastVictim(SPELL_GIFT_OF_THE_DOOMSAYER, true);
                    events.ScheduleEvent(EVENT_GIFT_OF_THE_DOOMSAYER, urand(16000, 21000));
                    break;
                case EVENT_WHIRLWIND:
                    DoCast(me, SPELL_WHIRLWIND);
                    Talk(SAY_WHIRLWIND);
                    events.ScheduleEvent(EVENT_WHIRLWIND, urand(19000, 21000));
                    events.ScheduleEvent(EVENT_HEAL, 6000);
                    break;
                case EVENT_HEAL:
                    DoCast(me, SPELL_HEAL);
                    Talk(SAY_HEAL);
                    break;
                case EVENT_SHADOW_WAVE:
                    DoCastVictim(SPELL_SHADOW_WAVE, true);
                    events.ScheduleEvent(EVENT_SHADOW_WAVE, urand(11000, 16000));
                    break;
                case EVENT_ME_FIRST:
                    if (Creature* soccothrates = ObjectAccessor::GetCreature(*me, instance->GetGuidData(DATA_SOCCOTHRATES)))
                        if (soccothrates->isAlive() && !soccothrates->isInCombat())
                            soccothrates->AI()->Talk(SAY_AGGRO_DALLIAH_FIRST);
                    break;
                default:
                    break;
                }

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;
            }

            if (HealthBelowPct(25) && !soccothratesTaunt)
            {
                if (Creature* soccothrates = ObjectAccessor::GetCreature(*me, instance->GetGuidData(DATA_SOCCOTHRATES)))
                    soccothrates->AI()->Talk(SAY_DALLIAH_25_PERCENT);
                soccothratesTaunt = true;
            }

            DoMeleeAttackIfReady();
        }

    private:
        bool soccothratesTaunt;
        bool soccothratesDeath;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_dalliah_the_doomsayerAI(creature);
    }
};

void AddSC_boss_dalliah_the_doomsayer()
{
    new boss_dalliah_the_doomsayer();
}