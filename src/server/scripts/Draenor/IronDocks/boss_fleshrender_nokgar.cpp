/*
    Dungeon : Iron Docks 93-95
    Encounter: Fleshrender Nok'gar
*/

#include "iron_docks.h"

enum Says
{
    SAY_AGGRO                 = 0,
    SAY_BURNING_ARROWS        = 1,
    SAY_BARBED_ARROW_BARRAGE  = 2,
    SAY_RECKLESS_PROVOCATION  = 3,
    SAY_WARN_RECKLESS         = 4,
    SAY_KILLED_PLAYER         = 5,
    SAY_DEATH                 = 6
};

enum Spells
{
    //Nokgar
    SPELL_SUMMON_DREADFANG      = 164049,
    SPELL_RIDE_VEHICLE          = 46598,
    SPELL_RECKLESS_PROVOCATION  = 164426,
    SPELL_BURNING_ARROWS        = 164635,
    SPELL_BURNING_ARROWS_SELECT = 166186,
    SPELL_BURNING_ARROWS_AT     = 164234,
    SPELL_BARBED_ARROW_BARRAGE  = 166923,
    SPELL_BARBED_ARROW_SELECT   = 166914,

    //Dreadfang
    SPELL_SAVAGE_MAULING        = 166290,
    SPELL_SAVAGE_MAULING_AURA   = 164837,
    SPELL_BLOODLETTING_HOWL     = 164835,
    SPELL_SHREDDING_SWIPES      = 164730,
    SPELL_SHREDDING_SWIPES_AT   = 164733,
    SPELL_SHREDDING_SWIP_REMOVE = 164735
};

enum eEvents
{
    //Nokgar
    EVENT_RIDE_VEHICLE              = 1,
    EVENT_RECKLESS_PROVOCATION      = 2,
    EVENT_BURNING_ARROWS            = 3,
    EVENT_BARBED_ARROWS             = 4,

    //Dreadfang
    EVENT_SAVAGE_MAULING            = 1,
    EVENT_SHREDDING_SWIPES          = 2,
    EVENT_BLOODLETTING_HOWL         = 3
};

struct boss_fleshrender_nokgar : public BossAI
{
    explicit boss_fleshrender_nokgar(Creature* creature) : BossAI(creature, DATA_NOKGAR)
    {
        first = false;
    }

    ObjectGuid dreadfangGUID;
    bool first;

    void Reset() override
    {
        _Reset();
        summons.DespawnAll();
        events.Reset();

        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_PC);

        if (!first)
            DoCast(SPELL_SUMMON_DREADFANG);
    }

    void JustReachedHome() override
    {
        DoCast(SPELL_SUMMON_DREADFANG);
    }

    void EnterCombat(Unit* /*who*/) override
    {
        first = true;
        Talk(SAY_AGGRO);
        _EnterCombat();

        events.RescheduleEvent(EVENT_RECKLESS_PROVOCATION, 34000);
        events.RescheduleEvent(EVENT_BURNING_ARROWS, 16000);
        events.RescheduleEvent(EVENT_BARBED_ARROWS, 44000);
    }

    void EnterEvadeMode() override
    {
        BossAI::EnterEvadeMode();
    }

    void KilledUnit(Unit* victim) override
    {
        if (victim->IsPlayer())
            Talk(SAY_KILLED_PLAYER);
    }

    void JustDied(Unit* /*killer*/) override
    {
        Talk(SAY_DEATH);
        _JustDied();

        if (instance->GetData(DATA_CAPTAIN_TEXT_1) != DONE)
        {
            if (auto skulloc = instance->instance->GetCreature(instance->GetGuidData(NPC_SKULLOC)))
                skulloc->CastSpell(skulloc, SPELL_IRON_DOCKS_BANTER_1, true);

            instance->SetData(DATA_CAPTAIN_TEXT_1, DONE);
        }
    }

    void JustSummoned(Creature* summon) override
    {
        summons.Summon(summon);

        dreadfangGUID.Clear();
        dreadfangGUID = summon->GetGUID();

        events.RescheduleEvent(EVENT_RIDE_VEHICLE, 500);
    }

    void SpellHitTarget(Unit* target, SpellInfo const* spell) override
    {
        if (target->GetEntry() == NPC_GROMKAR_FLAMESLINGER)
        {
            if (spell->Id == SPELL_BURNING_ARROWS_SELECT)
                if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                    target->CastSpell(target, SPELL_BURNING_ARROWS_AT);

            if (spell->Id == SPELL_BARBED_ARROW_BARRAGE)
                if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                    target->CastSpell(target, SPELL_BARBED_ARROW_SELECT);
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim() && me->isInCombat())
            return;

        if (me->GetDistance(me->GetHomePosition()) >= 90.0f)
        {
            EnterEvadeMode();
            return;
        }

        events.Update(diff);

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_RIDE_VEHICLE:
                if (auto dreadfang = ObjectAccessor::GetCreature(*me, dreadfangGUID))
                    DoCast(dreadfang, SPELL_RIDE_VEHICLE, true);
                break;
            case EVENT_RECKLESS_PROVOCATION:
                if (!me->HasAura(SPELL_RIDE_VEHICLE))
                {
                    Talk(SAY_WARN_RECKLESS);
                    Talk(SAY_RECKLESS_PROVOCATION);
                    DoCast(SPELL_RECKLESS_PROVOCATION);
                }
                events.RescheduleEvent(EVENT_RECKLESS_PROVOCATION, 42000);
                break;
            case EVENT_BURNING_ARROWS:
                Talk(SAY_BURNING_ARROWS);
                DoCast(SPELL_BURNING_ARROWS);
                events.RescheduleEvent(EVENT_BURNING_ARROWS, 18000);
                break;
            case EVENT_BARBED_ARROWS:
                Talk(SAY_BARBED_ARROW_BARRAGE);
                DoCast(SPELL_BARBED_ARROW_BARRAGE);
                events.RescheduleEvent(EVENT_BARBED_ARROWS, 42000);
                break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

// 81297
struct npc_dreadfang : public ScriptedAI
{
    explicit npc_dreadfang(Creature* creature) : ScriptedAI(creature), summons(me)
    {
        instance = creature->GetInstanceScript();
        exitPassenger = false;
    }

    InstanceScript* instance;
    SummonList summons;
    EventMap events;

    bool exitPassenger;

    void Reset() override
    {
        events.Reset();
        me->RemoveAllAuras();
        me->SetReactState(REACT_AGGRESSIVE);
        summons.DespawnEntry(NPC_SHREDDING_SWIPES);

        DoCast(164024);
    }

    void EnterCombat(Unit* /*who*/) override
    {
        EnterEvadeModeOwner();
        instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);

        events.RescheduleEvent(EVENT_SAVAGE_MAULING, 28000);
        events.RescheduleEvent(EVENT_SHREDDING_SWIPES, 36000);
        events.RescheduleEvent(EVENT_BLOODLETTING_HOWL, 44000);
    }

    void JustDied(Unit* /*who*/) override
    {
        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
    }

    void EnterEvadeModeOwner()
    {
        if (auto nokgar = me->GetAnyOwner())
            nokgar->ToCreature()->AI()->DoZoneInCombat();
    }

    void EnterEvadeMode() override
    {
        ScriptedAI::EnterEvadeMode();
        EnterEvadeModeOwner();
    }

    void DamageTaken(Unit* /*attacker*/, uint32& /*damage*/, DamageEffectType /*dmgType*/) override
    {
        if (me->HealthBelowPct(50) && !exitPassenger)
        {
            exitPassenger = true;
            me->GetVehicleKit()->RemoveAllPassengers();

            if (auto nokgar = me->GetAnyOwner())
                nokgar->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_PC);
        }
    }

    void MovementInform(uint32 type, uint32 id) override
    {
        if (type == POINT_MOTION_TYPE)
        {
            summons.DespawnEntry(NPC_SHREDDING_SWIPES);
            me->SetReactState(REACT_AGGRESSIVE);
            DoCast(SPELL_SHREDDING_SWIP_REMOVE);
        }
    }

    void JustSummoned(Creature* summon) override
    {
        summons.Summon(summon);

        if (summon->GetEntry() == NPC_SHREDDING_SWIPES)
            me->GetMotionMaster()->MovePoint(1, summon->GetPositionX(), summon->GetPositionY(), summon->GetPositionZ());
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        if (me->GetDistance(me->GetHomePosition()) >= 90.0f)
        {
            EnterEvadeMode();
            return;
        }

        events.Update(diff);

        if (auto victim = me->getVictim())
            if (victim->HasAura(SPELL_SAVAGE_MAULING_AURA))
                return;

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_SAVAGE_MAULING:
                if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 40.0f, true))
                {
                    me->AddThreat(target, 10000.0f);
                    DoCast(target, SPELL_SAVAGE_MAULING, false);
                }
                events.RescheduleEvent(EVENT_SAVAGE_MAULING, 10000);
                break;
            case EVENT_SHREDDING_SWIPES:
                me->StopAttack();
                DoCast(SPELL_SHREDDING_SWIPES);
                events.RescheduleEvent(EVENT_SHREDDING_SWIPES, 36000);
                break;
            case EVENT_BLOODLETTING_HOWL:
                DoCast(SPELL_BLOODLETTING_HOWL);
                events.RescheduleEvent(EVENT_BLOODLETTING_HOWL, 44000);
                break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

void AddSC_boss_fleshrender_nokgar()
{
    RegisterCreatureAI(boss_fleshrender_nokgar);
    RegisterCreatureAI(npc_dreadfang);
}
