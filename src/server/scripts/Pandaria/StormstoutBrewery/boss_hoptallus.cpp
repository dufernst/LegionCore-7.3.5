/*====================
======================*/

#include "stormstout_brewery.h"

enum Spells
{
    SPELL_CARROT_BREATH             = 112944,
    SPELL_FURL_WIND                 = 112992, 
    SPELL_EXPLOSIVE_BREW            = 114291,
    SPELL_EXPLOSIVE_BREW_JUMP_DMG   = 116027
};

enum eEvents
{
    EVENT_WP_1      = 1,
    EVENT_WP_2      = 2,
    EVENT_WP_3      = 3,
    EVENT_WP_4      = 4
};

const Position ePos[6] =
{
    {-711.41f, 1314.26f, 163.0f},
    {-752.30f, 1326.73f, 163.0f},
    {-769.08f, 1298.62f, 163.0f},
    {-768.20f, 1273.40f, 163.0f},
    {-758.04f, 1236.80f, 163.0f},
    {-775.34f, 1215.79f, 169.0f}
};

struct boss_hoptallus : public BossAI
{
    explicit boss_hoptallus(Creature* creature) : BossAI(creature, DATA_HOPTALLUS) {}

    uint32 breathtimer;
    uint32 windtimer;
    uint32 summonadd;

    void Reset() override
    {
        _Reset();
        breathtimer = 0;
        windtimer = 0;
        summonadd = 0;
    }

    void EnterCombat(Unit* /*who*/) override
    {
        _EnterCombat();
        breathtimer = 13000;
        windtimer = 8000;
        summonadd = 10000;
    }

    void JustSummoned(Creature* summon) override
    {
        summons.Summon(summon);
    }

    void JustDied(Unit* /*killer*/) override
    {
        _JustDied();
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        if (breathtimer <= diff)
        {
            DoCast(SPELL_CARROT_BREATH);
            breathtimer = 13000;
        }
        else
            breathtimer -= diff;

        if (windtimer <= diff)
        {
            DoCast(SPELL_FURL_WIND);
            windtimer = 8000;
        }
        else
            windtimer -= diff;

        if (summonadd <= diff)
        {
            for (uint8 i = 0; i < 3; i++)
                if (auto hopper = me->SummonCreature(59464, me->GetPositionX() + 5, me->GetPositionY(), me->GetPositionZ(), TEMPSUMMON_CORPSE_DESPAWN))
                    hopper->SetInCombatWithZone();

            summonadd = 10000;
        }
        else
            summonadd -= diff;

        DoMeleeAttackIfReady();
    }
};

struct npc_hopper : public ScriptedAI
{
    explicit npc_hopper(Creature* creature) : ScriptedAI(creature)
    {
        move = false;
    }

    bool explose;
    bool move;
    EventMap events;

    void Reset() override
    {
        explose = false;
        me->SetReactState(REACT_AGGRESSIVE);
    }

    void EnterEvadeMode() override
    {
        me->DespawnOrUnsummon();
    }

    void IsSummonedBy(Unit* owner) override
    {
        if (owner->GetEntry() == NPC_TRIGGER_SUMMONER)
        {
            move = true;
            me->setFaction(14);
            me->SetWalk(true);
            me->GetMotionMaster()->MovePoint(1, ePos[0]);
        }
    }

    void MovementInform(uint32 type, uint32 id) override
    {
        if (type == POINT_MOTION_TYPE)
        {
            switch (id)
            {
            case 1:
                events.RescheduleEvent(EVENT_WP_1, 0);
                break;
            case 2:
                events.RescheduleEvent(EVENT_WP_2, 0);
                break;
            case 3:
                events.RescheduleEvent(EVENT_WP_3, 0);
                break;
            case 4:
                events.RescheduleEvent(EVENT_WP_4, 0);
                break;
            case 5:
                me->GetMotionMaster()->MoveJump(ePos[5].GetPositionX() + irand(-5, 5), ePos[5].GetPositionY() + irand(-5, 5), ePos[5].GetPositionZ(), 15, 15);
                me->DespawnOrUnsummon();
                break;
            }
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim() && !move)
            return;

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        events.Update(diff);
        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_WP_1:
                me->GetMotionMaster()->MovePoint(2, ePos[1]);
                break;
            case EVENT_WP_2:
                me->GetMotionMaster()->MovePoint(3, ePos[2]);
                break;
            case EVENT_WP_3:
                me->GetMotionMaster()->MovePoint(4, ePos[3]);
                break;
            case EVENT_WP_4:
                me->GetMotionMaster()->MovePoint(5, ePos[4]);
                break;
            }
        }

        if (auto victim = me->getVictim())
        {
            if (me->IsWithinMeleeRange(victim))
            {
                if (me->GetDistance(victim) <= 0.5f && !explose)
                {
                    explose = true;
                    DoCast(victim, SPELL_EXPLOSIVE_BREW, false);
                }
            }
        }
    }

    void SpellHit(Unit* /*caster*/, SpellInfo const* spell) override
    {
        if (spell->Id == SPELL_SMASH_DMG)
        {
            me->SetReactState(REACT_PASSIVE);
            me->AttackStop();
            me->InterruptSpell(CURRENT_GENERIC_SPELL);
            me->GetMotionMaster()->MoveJump(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ() + 20, 10, 18);
            DoCast(SPELL_EXPLOSIVE_BREW_JUMP_DMG);
        }
    }
};

void AddSC_boss_hoptallus()
{
    RegisterCreatureAI(boss_hoptallus);
    RegisterCreatureAI(npc_hopper);
}
