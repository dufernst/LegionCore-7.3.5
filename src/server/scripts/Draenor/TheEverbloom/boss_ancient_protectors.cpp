/*
    Dungeon : The Everbloom 100
    Encounter: Ancient Protectors
*/

#include "the_everbloom.h"

enum Says
{
    //Gola
    SAY_GOLA_AGGRO         = 1,
    SAY_GOLA_REVITALIZING  = 2,
    SAY_GOLA_RAPID_TIDES   = 3,
    SAY_GOLA_DEATH         = 6,

    //Telu
    SAY_TELU_BRAMBLE       = 2,
    SAY_TELU_DEATH         = 4
};

enum Spells
{
    //Life Warden Gola
    SPELL_WATER_COSMETIC_CHANNEL    = 173380,
    SPELL_WATER_BOLT                = 168092,
    SPELL_REVITALIZING_WATERS       = 168082,
    SPELL_RAPID_TIDES               = 168105,

    //Earthshaper Telu
    SPELL_NATURE_COSMETIC_CHANNEL   = 172325,
    SPELL_NATURE_WRATH              = 168040,
    SPELL_BRAMBLE_PATCH             = 177497,
    SPELL_BRIARSKIN                 = 168041,

    //Dulhu
    SPELL_RENDING_CHARGE            = 168186,
    SPELL_NOXIOUS_ERUPTION          = 175997,
    SPELL_GRASPING_VINE             = 168375,
    SPELL_GRASPING_VINE_VISUAL      = 168376,
    SPELL_GRASPING_VINE_LEAP        = 168378,
    SPELL_SLASH                     = 168383
};

enum eEvents
{
    //Life Warden Gola
    EVENT_WATER_BOLT        = 1,
    EVENT_HEAL_PCT          = 2,
    EVENT_RAPID_TIDES       = 3,

    //Earthshaper Telu
    EVENT_NATURE_WRATH      = 1,
    EVENT_BRAMBLE_PATCH     = 2,
    EVENT_BRIARSKIN         = 3,

    //Dulhu
    EVENT_RENDING_CHARGE    = 1,
    EVENT_NOXIOUS_ERUPTION  = 2,
    EVENT_GRASPING_VINE     = 3
};

uint32 const protectors[3] = { NPC_LIFE_WARDEN_GOLA, NPC_EARTHSHAPER_TELU, NPC_DULHU};

struct boss_encounter_ancient_protectors : public BossAI
{
    boss_encounter_ancient_protectors(Creature* creature) : BossAI(creature, DATA_PROTECTORS) {}

    uint8 bossDiedCount;

    void Reset() override
    {
        _Reset();
        events.Reset();
    }

    void EnterCombat(Unit* /*who*/) override
    {
        if (instance->GetBossState(DATA_PROTECTORS) != IN_PROGRESS)
            instance->SetBossState(DATA_PROTECTORS, IN_PROGRESS);
    }

    void JustDied(Unit* /*killer*/) override
    {
        bossDiedCount = 0;

        for (uint8 i = 0; i < 3; i++)
            if (auto enfor = ObjectAccessor::GetCreature(*me, instance->GetGuidData(protectors[i])))
                if (!enfor->isAlive())
                    bossDiedCount++;

        if (bossDiedCount == 3)
        {
            instance->SetBossState(DATA_PROTECTORS, DONE);
            instance->DoUpdateAchievementCriteria(CRITERIA_TYPE_SCRIPT_EVENT_2, 41761);
        }
    }
};

//83892
class boss_life_warden_gola : public CreatureScript
{
public:
    boss_life_warden_gola() : CreatureScript("boss_life_warden_gola") {}

    struct boss_life_warden_golaAI : public boss_encounter_ancient_protectors
    {
        boss_life_warden_golaAI(Creature* creature) : boss_encounter_ancient_protectors(creature) {}

        ObjectGuid targetGUID;

        void Reset() override
        {
            targetGUID.Clear();
            boss_encounter_ancient_protectors::Reset();
            DoCast(SPELL_WATER_COSMETIC_CHANNEL);
        }

        void EnterCombat(Unit* who) override
        {
            boss_encounter_ancient_protectors::EnterCombat(who);
            instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
            Talk(SAY_GOLA_AGGRO);
            events.RescheduleEvent(EVENT_WATER_BOLT, 0); 
            events.RescheduleEvent(EVENT_HEAL_PCT, 18000);
            events.RescheduleEvent(EVENT_RAPID_TIDES, 22000);
        }

        void JustDied(Unit* /*killer*/) override
        {
            boss_encounter_ancient_protectors::JustDied(NULL);
            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
            Talk(SAY_GOLA_DEATH);
        }

        void SpellHit(Unit* /*caster*/, SpellInfo const* spell) override
        {
            if (spell->Id == SPELL_RAPID_TIDES)
            {
                events.RescheduleEvent(EVENT_HEAL_PCT, 2000);
                events.RescheduleEvent(EVENT_RAPID_TIDES, 2000);
            }
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
                    case EVENT_WATER_BOLT:
                        if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                        {
                            DoResetThreat();
                            me->AddThreat(target, 10000.0f);
                            DoCast(target, SPELL_WATER_BOLT, false);
                        }
                        events.RescheduleEvent(EVENT_WATER_BOLT, 3100);
                        break;
                    case EVENT_HEAL_PCT:
                    {
                        auto targetA = instance->instance->GetCreature(instance->GetGuidData(NPC_EARTHSHAPER_TELU));
                        auto targetB = instance->instance->GetCreature(instance->GetGuidData(NPC_DULHU));
                        if (!targetA || !targetB)
                            return;

                        if (targetA->isAlive() && targetB->isAlive())
                            targetGUID = targetA->GetHealthPct() < targetB->GetHealthPct() ? targetA->GetGUID() : targetB->GetGUID();
                        else if (!targetA->isAlive() && !targetB->isAlive())
                            targetGUID = me->GetGUID();
                        else if (!targetA->isAlive() || !targetB->isAlive())
                            targetGUID = targetA->isAlive() ? targetA->GetGUID() : targetB->GetGUID();

                        if (auto target = ObjectAccessor::GetCreature(*me, targetGUID))
                            DoCast(target, SPELL_REVITALIZING_WATERS, false);

                        if (me->HasAura(SPELL_RAPID_TIDES))
                            events.RescheduleEvent(EVENT_HEAL_PCT, 2000);
                        else
                        {
                            Talk(SAY_GOLA_REVITALIZING);
                            events.RescheduleEvent(EVENT_HEAL_PCT, 22000);
                        }
                        break;
                    }
                    case EVENT_RAPID_TIDES:
                        DoCast(SPELL_RAPID_TIDES);

                        if (me->HasAura(SPELL_RAPID_TIDES))
                            events.RescheduleEvent(EVENT_RAPID_TIDES, 2000);
                        else
                        {
                            Talk(SAY_GOLA_RAPID_TIDES);
                            events.RescheduleEvent(EVENT_RAPID_TIDES, 22000);
                        }
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_life_warden_golaAI (creature);
    }
};

//83893
class boss_earthshaper_telu : public CreatureScript
{
public:
    boss_earthshaper_telu() : CreatureScript("boss_earthshaper_telu") {}

    struct boss_earthshaper_teluAI : public boss_encounter_ancient_protectors
    {
        boss_earthshaper_teluAI(Creature* creature) : boss_encounter_ancient_protectors(creature) {}

        ObjectGuid targetGUID;

        void Reset() override
        {
            targetGUID.Clear();
            boss_encounter_ancient_protectors::Reset();
            DoCast(SPELL_NATURE_COSMETIC_CHANNEL);
        }

        void EnterCombat(Unit* who) override
        {
            boss_encounter_ancient_protectors::EnterCombat(who);
            instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
            events.RescheduleEvent(EVENT_NATURE_WRATH, 0);
            events.RescheduleEvent(EVENT_BRAMBLE_PATCH, 12000);
            events.RescheduleEvent(EVENT_BRIARSKIN, 26000);
        }

        void JustDied(Unit* /*killer*/) override
        {
            boss_encounter_ancient_protectors::JustDied(NULL);
            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
            Talk(SAY_TELU_DEATH);
        }

        void SpellHit(Unit* /*caster*/, SpellInfo const* spell) override
        {
            if (spell->Id == SPELL_RAPID_TIDES)
            {
                events.RescheduleEvent(EVENT_BRAMBLE_PATCH, 2000);
                events.RescheduleEvent(EVENT_BRIARSKIN, 2000);
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
                    case EVENT_NATURE_WRATH:
                        if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                        {
                            DoResetThreat();
                            me->AddThreat(target, 10000.0f);
                            DoCast(target, SPELL_NATURE_WRATH, false);
                        }
                        events.RescheduleEvent(EVENT_NATURE_WRATH, 3100);
                        break;
                    case EVENT_BRAMBLE_PATCH:
                        DoCast(SPELL_BRAMBLE_PATCH);

                        if (me->HasAura(SPELL_RAPID_TIDES))
                            events.RescheduleEvent(EVENT_BRAMBLE_PATCH, 2000);
                        else
                        {
                            Talk(SAY_TELU_BRAMBLE);
                            events.RescheduleEvent(EVENT_BRAMBLE_PATCH, 20000);
                        }
                        break;
                    case EVENT_BRIARSKIN:
                        auto targetA = instance->instance->GetCreature(instance->GetGuidData(NPC_LIFE_WARDEN_GOLA));
                        auto targetB = instance->instance->GetCreature(instance->GetGuidData(NPC_DULHU));
                        if (!targetA || !targetB)
                            return;

                        if (targetA->isAlive() && targetB->isAlive())
                            targetGUID = targetA->GetHealthPct() < targetB->GetHealthPct() ? targetA->GetGUID() : targetB->GetGUID();
                        else if (!targetA->isAlive() && !targetB->isAlive())
                            targetGUID = me->GetGUID();
                        else if (!targetA->isAlive() || !targetB->isAlive())
                            targetGUID = targetA->isAlive() ? targetA->GetGUID() : targetB->GetGUID();

                        if (auto target = ObjectAccessor::GetCreature(*me, targetGUID))
                            DoCast(target, SPELL_BRIARSKIN);

                        if (me->HasAura(SPELL_RAPID_TIDES))
                            events.RescheduleEvent(EVENT_BRIARSKIN, 2000);
                        else
                            events.RescheduleEvent(EVENT_BRIARSKIN, 22000);
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_earthshaper_teluAI (creature);
    }
};

//83894
class boss_dulhu : public CreatureScript
{
public:
    boss_dulhu() : CreatureScript("boss_dulhu") {}

    struct boss_dulhuAI : public boss_encounter_ancient_protectors
    {
        boss_dulhuAI(Creature* creature) : boss_encounter_ancient_protectors(creature) {}

        void Reset() override
        {
            boss_encounter_ancient_protectors::Reset();
        }

        void EnterCombat(Unit* who) override
        {
            boss_encounter_ancient_protectors::EnterCombat(who);
            instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
            events.RescheduleEvent(EVENT_RENDING_CHARGE, 6000);
            events.RescheduleEvent(EVENT_NOXIOUS_ERUPTION, 7000);
            events.RescheduleEvent(EVENT_GRASPING_VINE, 22000);
        }

        void JustDied(Unit* /*killer*/) override
        {
            me->RemoveFlag(OBJECT_FIELD_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);
            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
            boss_encounter_ancient_protectors::JustDied(NULL);
        }

        void SpellHit(Unit* /*caster*/, SpellInfo const* spell) override
        {
            if (spell->Id == SPELL_RAPID_TIDES)
            {
                events.RescheduleEvent(EVENT_NOXIOUS_ERUPTION, 2000);
                events.RescheduleEvent(EVENT_GRASPING_VINE, 2000);
            }
        }

        void SpellHitTarget(Unit* target, SpellInfo const* spell) override
        {
            if (spell->Id == SPELL_GRASPING_VINE_VISUAL)
            {
                target->CastSpell(me, SPELL_GRASPING_VINE_LEAP, true);
                DoCast(target, SPELL_SLASH, false);
            }
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
                    case EVENT_RENDING_CHARGE:
                        if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 60, true))
                            DoCast(target, SPELL_RENDING_CHARGE, false);

                        events.RescheduleEvent(EVENT_RENDING_CHARGE, 20000);
                        break;
                    case EVENT_NOXIOUS_ERUPTION:
                        DoCast(SPELL_NOXIOUS_ERUPTION);

                        if (me->HasAura(SPELL_RAPID_TIDES))
                            events.RescheduleEvent(EVENT_NOXIOUS_ERUPTION, 2000);
                        else
                            events.RescheduleEvent(EVENT_NOXIOUS_ERUPTION, 18000);
                        break;
                    case EVENT_GRASPING_VINE:
                        DoCast(SPELL_GRASPING_VINE);

                        if (me->HasAura(SPELL_RAPID_TIDES))
                            events.RescheduleEvent(EVENT_GRASPING_VINE, 2000);
                        else
                            events.RescheduleEvent(EVENT_GRASPING_VINE, 22000);
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_dulhuAI (creature);
    }
};

void AddSC_boss_ancient_protectors()
{
    new boss_life_warden_gola();
    new boss_earthshaper_telu();
    new boss_dulhu();
}
