#include"the_vortex_pinnacle.h"
#include "ScriptedEscortAI.h"

enum ScriptTexts
{
    SAY_AGGRO   = 0,
    SAY_KILL    = 1,
    SAY_DEATH   = 2
};

enum Spells
{
    SPELL_CYCLONE_SHIELD        = 86267,
    SPELL_CYCLONE_SHIELD_DMG    = 86292,
    SPELL_SUMMON_TEMPEST        = 86340,
    SPELL_STORM_EDGE            = 86309,
    SPELL_LIGHTNING_BOLT        = 86331
};

enum Events
{
    EVENT_LIGHTNING_BOLT    = 1,
    EVENT_STORM_EDGE        = 2,
    EVENT_CALL_VORTEX       = 3,
    EVENT_RESET_VORTEX      = 4,
    EVENT_SUMMON_TEMPEST    = 5
};

enum Adds
{
    NPC_ERTAN_VORTEX    = 46007,
    NPC_SLIPSTREAM      = 45455
};

const Position ertanvortexPos_1[8] = 
{
    { -702.109985f, -13.500000f, 635.669983f, 0.0f },
    { -719.549988f, -21.190001f, 635.669983f, 0.0f },
    { -737.419983f, -13.970000f, 635.669983f, 0.0f },
    { -745.000000f, 3.990000f,635.669983f, 0.0f },
    { -737.650024f, 21.790001f, 635.669983f, 0.0f },
    { -720.190002f, 29.540001f, 635.669983f, 0.0f },
    { -702.070007f, 22.150000f, 635.669983f, 0.0f },
    { -694.539978f, 4.250000f,635.669983f, 0.0f }
};

struct boss_grand_vizier_ertan : public BossAI
{
    explicit boss_grand_vizier_ertan(Creature* creature) : BossAI(creature, DATA_ERTAN)
    {
        me->setActive(true);
    }

    ObjectGuid _vortexes[8];
    float _distance;

    void Reset() override
    {
        _Reset();
        memset(_vortexes, NULL, sizeof(_vortexes));
        me->SetReactState(REACT_AGGRESSIVE);
        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
    }

    void EnterCombat(Unit* /*who*/) override
    {
        for (uint8 i = 0; i < 8; i++)
        {
            if (auto creature = me->SummonCreature(NPC_ERTAN_VORTEX, ertanvortexPos_1[i]))
            {
                _vortexes[i] = creature->GetGUID();
                creature->AI()->DoAction(i);
            }
        }

        events.ScheduleEvent(EVENT_LIGHTNING_BOLT, 3000);
        events.ScheduleEvent(EVENT_CALL_VORTEX, urand(18000, 21000));
        events.ScheduleEvent(EVENT_STORM_EDGE, 5000);

        if (IsHeroic() || me->GetMap()->GetDifficultyID() == DIFFICULTY_TIMEWALKING)
            events.ScheduleEvent(EVENT_SUMMON_TEMPEST, 16000);

        Talk(SAY_AGGRO);
        DoZoneInCombat();
        _EnterCombat();
    }

    void KilledUnit(Unit* victim) override
    {
        if (!victim)
            return;

        if (victim->IsPlayer())
            Talk(SAY_KILL);
    }

    void JustDied(Unit* /*pWho*/) override
    {
        _JustDied();
        Talk(SAY_DEATH);
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
            case EVENT_LIGHTNING_BOLT:
                if (auto victim = me->getVictim())
                    DoCast(victim, SPELL_LIGHTNING_BOLT, false);

                events.ScheduleEvent(EVENT_LIGHTNING_BOLT, 2000);
                break;
            case EVENT_CALL_VORTEX:
                for (uint8 i = 0; i < 8; i++)
                    if (_vortexes[i])
                        if (auto creature = ObjectAccessor::GetCreature(*me, _vortexes[i]))
                            creature->AI()->DoAction(15);

                events.ScheduleEvent(EVENT_RESET_VORTEX, urand(14000, 17000));
                break;
            case EVENT_RESET_VORTEX:
                for (uint8 i = 0; i < 8; i++)
                    if (_vortexes[i])
                        if (auto creature = ObjectAccessor::GetCreature(*me, _vortexes[i]))
                            creature->AI()->DoAction(16);

                events.ScheduleEvent(EVENT_CALL_VORTEX, urand(20000, 25000));
                break;
            case EVENT_STORM_EDGE:
                if (auto creature = ObjectAccessor::GetCreature(*me, _vortexes[1]))
                    _distance = me->GetDistance2d(creature);

                instance->instance->ApplyOnEveryPlayer([&](Player* player)
                {
                    if (me->GetDistance2d(player) > _distance)
                    {
                        uint8 i = urand(0, 7);
                        if (auto creature = ObjectAccessor::GetCreature(*me, _vortexes[i]))
                            creature->CastSpell(player, SPELL_STORM_EDGE, true);

                        DoCast(player, SPELL_STORM_EDGE, true);
                    }
                });

                events.ScheduleEvent(EVENT_STORM_EDGE, 2000);
                break;
            case EVENT_SUMMON_TEMPEST:
                DoCast(SPELL_SUMMON_TEMPEST);
                events.ScheduleEvent(EVENT_SUMMON_TEMPEST, 18000);
                break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

class npc_ertan_vortex : public CreatureScript
{
public:
    npc_ertan_vortex() : CreatureScript("npc_ertan_vortex") {}

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_ertan_vortexAI(pCreature);
    }
    struct npc_ertan_vortexAI : public npc_escortAI
    {
        explicit npc_ertan_vortexAI(Creature* pCreature) : npc_escortAI(pCreature)
        {
            Reset();
        }

        EventMap events;
        uint8 _mobid;
        ObjectGuid _owner;
        float x, y, z;

        void Reset() override
        {
            x = y = z = 0.0f;
            _mobid = 0;
            DoCast(me, SPELL_CYCLONE_SHIELD);
            events.Reset();
        }

        void InitWaypoint()
        {
            AddWaypoint(0, -702.109985f, -13.500000f, 635.669983f);
            AddWaypoint(1, -702.109985f, -13.500000f, 635.669983f);
            AddWaypoint(2, -719.549988f, -21.190001f, 635.669983f);
            AddWaypoint(3, -737.419983f, -13.970000f, 635.669983f);
            AddWaypoint(4, -745.000000f, 3.990000f, 635.669983f);
            AddWaypoint(5, -737.650024f, 21.790001f, 635.669983f);
            AddWaypoint(6, -720.190002f, 29.540001f, 635.669983f);
            AddWaypoint(7, -702.070007f, 22.150000f, 635.669983f);
            AddWaypoint(8, -694.539978f, 4.250000f, 635.669983f);
        }

        void WaypointReached(uint32 i) override
        {
            if (i == 8)
                SetCurentWP(0);
        }

        void IsSummonedBy(Unit* owner) override
        {
            _owner = owner->GetGUID();
        }

        void DoAction(int32 const action) override
        {
            if (action < 10)
            {
                _mobid = action + 1;
                InitWaypoint();
                Start(false, false);
                SetCurentWP(_mobid);
            }
            else if (action == 15)
            {
                SetEscortPaused(true);
                events.ScheduleEvent(EVENT_CALL_VORTEX, 1000);
            }
            else if (action == 16)
            {
                SetEscortPaused(true);
                events.ScheduleEvent(EVENT_RESET_VORTEX, 1000);
            }
        }

        void UpdateAI(uint32 diff) override
        {
            events.Update(diff);
            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_CALL_VORTEX:
                    if (Unit* owner = ObjectAccessor::GetUnit(*me, _owner))
                    {
                        float _angle;
                        Position _pos;
                        x = me->GetPositionX();
                        y = me->GetPositionY();
                        z = me->GetPositionZ();
                        _angle = owner->GetAngle(me->GetPositionX(), me->GetPositionY());
                        owner->GetNearPosition(_pos, 5.0f, _angle);
                        me->GetMotionMaster()->MovementExpired(false);
                        me->GetMotionMaster()->MovePoint(GetCurentWP(), _pos);
                    }
                    break;
                case EVENT_RESET_VORTEX:
                    SetEscortPaused(false);
                    break;
                }
            }
            npc_escortAI::UpdateAI(diff);
        }
    };
};

void AddSC_boss_grand_vizier_ertan()
{
    RegisterCreatureAI(boss_grand_vizier_ertan);
    new npc_ertan_vortex();
}