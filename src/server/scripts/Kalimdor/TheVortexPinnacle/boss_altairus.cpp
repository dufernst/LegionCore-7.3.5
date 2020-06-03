#include"the_vortex_pinnacle.h"

enum Spells
{
    SPELL_CALL_OF_WIND              = 88244,
    SPELL_CALL_OF_WIND_DUMMY_1      = 88276,
    SPELL_CALL_OF_WIND_DUMMY_2      = 88772,
    SPELL_DOWNWIND_OF_ALTAIRUS      = 88286,
    SPELL_UPWIND_OF_ALTAIRUS        = 88282,
    SPELL_CHILLING_BREATH           = 88308,
    SPELL_CHILLING_BREATH_DUMMY     = 88322,
    SPELL_LIGHTNING_BLAST           = 88357,
    SPELL_LIGHTNING_BLAST_DUMMY     = 88332,
    SPELL_TWISTER_AURA              = 88313,
    SPELL_TWISTER_AURA_DMG          = 88314
    
};

enum Events
{
    EVENT_CALL_OF_WIND          = 1,
    EVENT_CHILLING_BREATH       = 2,
    EVENT_LIGHTNING_BLAST       = 3,
    EVENT_CHECK_FACING          = 4,
    EVENT_RESET_WIND            = 5
};

enum Adds
{
    NPC_TWISTER        = 47342,
    NPC_AIR_CURRENT    = 47305
};

const float orientations[4] = {5.70f, 2.54f, 0.84f, 4.44f};

struct boss_altairus : public BossAI
{
    explicit boss_altairus(Creature* creature) : BossAI(creature, DATA_ALTAIRUS)
    {
        me->setActive(true);
    }

    uint8 _twisternum;

    void Reset() override
    {
        _Reset();
        _twisternum = 0;
    }

    void EnterCombat(Unit* /*who*/) override
    {
        events.ScheduleEvent(EVENT_CHILLING_BREATH, urand(5000, 10000));
        events.ScheduleEvent(EVENT_CALL_OF_WIND, 2000);
        events.ScheduleEvent(EVENT_CHECK_FACING, 2500);
        DoZoneInCombat();
        instance->SetBossState(DATA_ALTAIRUS, IN_PROGRESS);
    }

    void JustDied(Unit* /*who*/) override
    {
        _JustDied();
    }

    bool CheckOrientation(float player, float creature)
    {
        float _cur, _up, _down;

        if (creature > M_PI)
            _cur = creature - M_PI;
        else
            _cur = creature + M_PI;


        _up = _cur + 1.0f;
        _down = _cur - 1.0f;

        if (player > _down && player < _up)
            return true;
        else
            return false;
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        if (auto victim = me->getVictim())
        {
            if (victim->GetDistance2d(me->GetHomePosition().GetPositionX(), me->GetHomePosition().GetPositionY()) > 55.0f)
            {
                DoCast(victim, SPELL_LIGHTNING_BLAST, false);
                return;
            }
        }

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_CHILLING_BREATH:
                if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                    DoCast(target, SPELL_CHILLING_BREATH, false);

                events.ScheduleEvent(EVENT_CHILLING_BREATH, urand(10000, 16000));
                break;
            case EVENT_RESET_WIND:
                if (auto _aircurrent = me->FindNearestCreature(NPC_AIR_CURRENT, 100.0f))
                    _aircurrent->DespawnOrUnsummon();

                events.DelayEvents(1000);
                events.ScheduleEvent(EVENT_CALL_OF_WIND, 800);
                break;
            case EVENT_CALL_OF_WIND:
                me->SummonCreature(NPC_AIR_CURRENT, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), orientations[urand(0, 3)]);
                events.ScheduleEvent(EVENT_RESET_WIND, 18000);
                break;
            case EVENT_CHECK_FACING:
                if (auto _aircurrent = me->FindNearestCreature(NPC_AIR_CURRENT, 100.0f))
                {
                    if (me->GetMap()->GetPlayers().isEmpty() || !_aircurrent)
                        break;

                    for (Map::PlayerList::const_iterator itr = me->GetMap()->GetPlayers().begin(); itr != me->GetMap()->GetPlayers().end(); ++itr)
                    {
                        if (CheckOrientation(itr->getSource()->GetOrientation(), _aircurrent->GetOrientation()))
                        {
                            itr->getSource()->RemoveAurasDueToSpell(SPELL_DOWNWIND_OF_ALTAIRUS);
                            me->AddAura(SPELL_UPWIND_OF_ALTAIRUS, itr->getSource());
                        }
                        else
                        {
                            itr->getSource()->RemoveAurasDueToSpell(SPELL_UPWIND_OF_ALTAIRUS);
                            me->AddAura(SPELL_DOWNWIND_OF_ALTAIRUS, itr->getSource());
                        }
                    }

                    events.ScheduleEvent(EVENT_CHECK_FACING, 3000);
                    break;
                }
            }
        }
        DoMeleeAttackIfReady();
    }
};

struct npc_air_current : public ScriptedAI
{
    explicit npc_air_current(Creature* creature) : ScriptedAI(creature)
    {
        me->SetReactState(REACT_PASSIVE);
    }

    void Reset() override
    {
        DoCast(SPELL_CALL_OF_WIND_DUMMY_2);
    }
};

void AddSC_boss_altairus()
{
    RegisterCreatureAI(boss_altairus);
    RegisterCreatureAI(npc_air_current);
}