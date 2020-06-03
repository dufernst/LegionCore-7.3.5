#include"the_vortex_pinnacle.h"

enum ScriptTexts
{
    SAY_AGGRO   = 0,
    SAY_KILL    = 1,
    SAY_SPELL   = 2,
    SAY_DEATH   = 3
};

enum Spells
{
    SPELL_CHAIN_LIGHTNING                   = 87622,
    SPELL_UNSTABLE_GROUNDING_FIELD          = 86911,
    SPELL_GROUNDING_FIELD_VISUAL_BEAMS      = 87517,

    SPELL_SUPREMACY_OF_THE_STORM            = 86930,
    
    SPELL_SUPREMACY_OF_THE_STORM_DUMMY      = 86715,
    SPELL_SUPREMACY_OF_THE_STORM_DMG        = 87553,
    SPELL_SUPREMACY_OF_THE_STORM_DMG_H      = 93994,

    SPELL_SUPREMACY_OF_THE_STORM_DUMMY_1    = 87521,
    SPELL_SUPREMACY_OF_THE_STORM_SUM        = 87518,
    SPELL_SUPREMACY_OF_THE_STORM_TELE       = 87328,

    SPELL_STATIC_CLING                      = 87618
};

enum Events
{
    EVENT_FIELD             = 1,
    EVENT_FIELD_1           = 2,
    EVENT_FIELD_2           = 3,
    EVENT_FIELD_3           = 4,
    EVENT_FIELD_4           = 5,
    EVENT_FIELD_5           = 6,
    EVENT_FIELD_6           = 7,
    EVENT_CHAIN_LIGHTNING   = 8,
    EVENT_STATIC_CLING      = 9,
    EVENT_SUMMON_STAR       = 10
};

enum Adds
{
    NPC_UNSTABLE_GROUNDING_FIELD    = 46492,
    NPC_SKYFALL_STAR                = 52019,
    NPC_STORM_TARGET                = 46387,
    NPC_GROUNDING_FIELD             = 47000
};

const Position fieldPos[4] =
{
    { -644.20f, 489.00f, 646.63f, 0.0f },
    { -638.38f, 480.68f, 646.63f, 0.0f },
    { -635.43f, 492.11f, 646.63f, 0.0f },
    { -639.23f, 488.13f, 656.63f, 0.0f }
};

const Position starPos[6] = 
{
    { -583.77f, 516.56f, 649.51f, 5.65f },
    { -591.65f, 476.39f, 649.19f, 4.39f },
    { -617.69f, 544.79f, 650.12f, 0.11f },
    { -652.62f, 532.48f, 649.03f, 1.53f },
    { -618.65f, 463.05f, 650.63f, 0.0f },
    { -649.24f, 474.11f, 649.63f, 0.0f }
};

struct boss_asaad : public BossAI
{
    explicit boss_asaad(Creature* creature) : BossAI(creature, DATA_ASAAD)
    {
        me->setActive(true);
    }

    bool bField;
    ObjectGuid _field1GUID;
    ObjectGuid _field2GUID;
    ObjectGuid _field3GUID;

    void Reset() override
    {
        _Reset();
        me->SetCanFly(false);
        bField = false;
    }

    void EnterCombat(Unit* /*who*/) override
    {
        events.ScheduleEvent(EVENT_SUMMON_STAR, urand(10000, 20000));
        events.ScheduleEvent(EVENT_CHAIN_LIGHTNING, urand(5000, 10000));
        events.ScheduleEvent(EVENT_FIELD, 45000);

        if (IsHeroic())
            events.ScheduleEvent(EVENT_STATIC_CLING, urand(12000, 18000));

        Talk(SAY_AGGRO);
        bField = false;
        DoZoneInCombat();
        instance->SetBossState(DATA_ASAAD, IN_PROGRESS);
    }

    void KilledUnit(Unit* victim) override
    {
        if (!victim)
            return;

        if (victim->IsPlayer())
            Talk(SAY_KILL);
    }

    void JustDied(Unit* /*who*/) override
    {
        _JustDied();
        me->SetCanFly(false);
        Talk(SAY_DEATH);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING) && !bField)
            return;

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_CHAIN_LIGHTNING:
                if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                    DoCast(target, SPELL_CHAIN_LIGHTNING, false);

                events.ScheduleEvent(EVENT_CHAIN_LIGHTNING, urand(15000, 20000));
                break;
            case EVENT_SUMMON_STAR:
                me->SummonCreature(NPC_SKYFALL_STAR, starPos[urand(0, 5)]);
                events.ScheduleEvent(EVENT_SUMMON_STAR, urand(24000, 30000));
                break;
            case EVENT_STATIC_CLING:
                DoCast(SPELL_STATIC_CLING);
                events.ScheduleEvent(EVENT_STATIC_CLING, urand(20000, 23000));
                break;
            case EVENT_FIELD:
                bField = true;
                events.CancelEvent(EVENT_CHAIN_LIGHTNING);
                events.CancelEvent(EVENT_STATIC_CLING);
                events.CancelEvent(EVENT_SUMMON_STAR);
                me->InterruptNonMeleeSpells(false);
                Talk(SAY_SPELL);
                me->AttackStop();
                me->SetReactState(REACT_PASSIVE);

                if (auto sum1 = me->SummonCreature(NPC_UNSTABLE_GROUNDING_FIELD, fieldPos[0], TEMPSUMMON_MANUAL_DESPAWN, 0, _field1GUID))
                {
                    _field1GUID = sum1->GetGUID();

                    if (auto _field1 = ObjectAccessor::GetUnit(*me, _field1GUID))
                        DoCast(_field1, SPELL_GROUNDING_FIELD_VISUAL_BEAMS);
                }

                events.ScheduleEvent(EVENT_FIELD_4, 20000);
                events.ScheduleEvent(EVENT_FIELD_1, 6000);
                break;
            case EVENT_FIELD_1:
                if (auto sum2 = me->SummonCreature(NPC_UNSTABLE_GROUNDING_FIELD, fieldPos[1], TEMPSUMMON_MANUAL_DESPAWN, 0, _field2GUID))
                {
                    _field2GUID = sum2->GetGUID();

                    if (auto _field2 = ObjectAccessor::GetUnit(*me, _field2GUID))
                    {
                        if (auto _field1 = ObjectAccessor::GetUnit(*me, _field1GUID))
                            _field1->CastSpell(_field2, SPELL_GROUNDING_FIELD_VISUAL_BEAMS, true);
                    }
                }

                events.ScheduleEvent(EVENT_FIELD_2, 6000);
                break;
            case EVENT_FIELD_2:
                if (auto sum3 = me->SummonCreature(NPC_UNSTABLE_GROUNDING_FIELD, fieldPos[2], TEMPSUMMON_MANUAL_DESPAWN, 0, _field3GUID))
                {
                    _field3GUID = sum3->GetGUID();

                    if (auto _field2 = ObjectAccessor::GetUnit(*me, _field2GUID))
                    {
                        if (auto _field3 = ObjectAccessor::GetUnit(*me, _field3GUID))
                            _field2->CastSpell(_field3, SPELL_GROUNDING_FIELD_VISUAL_BEAMS, true);
                    }
                }

                events.ScheduleEvent(EVENT_FIELD_3, 6000);
                break;
            case EVENT_FIELD_3:
                if (auto _field3 = ObjectAccessor::GetUnit(*me, _field3GUID))
                {
                    if (auto _field1 = ObjectAccessor::GetUnit(*me, _field1GUID))
                        _field3->CastSpell(_field1, SPELL_GROUNDING_FIELD_VISUAL_BEAMS, true);
                }
                break;
            case EVENT_FIELD_4:
                me->SetCanFly(true);
                me->SetDisableGravity(true);
                me->NearTeleportTo(fieldPos[3].GetPositionX(), fieldPos[3].GetPositionY(), fieldPos[3].GetPositionZ(), 0.0f);
                events.ScheduleEvent(EVENT_FIELD_5, 800);
                break;
            case EVENT_FIELD_5:
                DoCast(SPELL_SUPREMACY_OF_THE_STORM);
                events.ScheduleEvent(EVENT_FIELD_6, 6000);
                break;
            case EVENT_FIELD_6:
                if (auto _field1 = ObjectAccessor::GetUnit(*me, _field1GUID))
                    _field1->ToCreature()->DespawnOrUnsummon();
                if (auto _field2 = ObjectAccessor::GetUnit(*me, _field2GUID))
                    _field2->ToCreature()->DespawnOrUnsummon();
                if (auto _field3 = ObjectAccessor::GetUnit(*me, _field3GUID))
                    _field3->ToCreature()->DespawnOrUnsummon();

                bField = false;
                me->SetCanFly(false);
                me->SetDisableGravity(false);;
                me->SetReactState(REACT_AGGRESSIVE);

                if (auto victim = me->getVictim())
                    AttackStart(me->getVictim());

                events.ScheduleEvent(EVENT_FIELD, 45000);
                events.ScheduleEvent(EVENT_SUMMON_STAR, urand(10000, 20000));
                events.ScheduleEvent(EVENT_CHAIN_LIGHTNING, urand(5000, 10000));

                if (IsHeroic())
                    events.ScheduleEvent(EVENT_STATIC_CLING, urand(12000, 18000));
                break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

struct npc_unstable_grounding_field : public ScriptedAI
{
    explicit npc_unstable_grounding_field(Creature* creature) : ScriptedAI(creature)
    {
        me->SetReactState(REACT_PASSIVE);
    }
};

class spell_asaad_supremacy_of_the_storm : public SpellScript
{
    PrepareSpellScript(spell_asaad_supremacy_of_the_storm);

    void HandleScript()
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster)
            return;
        if (!target)
            return;

        if (caster->GetDistance2d(target) < 5.0f)
            SetHitDamage(0);
    }

    void Register() override
    {
        BeforeHit += SpellHitFn(spell_asaad_supremacy_of_the_storm::HandleScript);
    }
};

void AddSC_boss_asaad()
{
    RegisterCreatureAI(boss_asaad);
    RegisterCreatureAI(npc_unstable_grounding_field);
    RegisterSpellScript(spell_asaad_supremacy_of_the_storm);
}