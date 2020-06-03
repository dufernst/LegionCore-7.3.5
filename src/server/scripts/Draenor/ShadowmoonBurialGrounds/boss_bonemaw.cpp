/*
    Dungeon : Shadowmoon Burial Grounds 100
    Encounter: Bonemaw
*/

#include "shadowmoon_burial_grounds.h"

enum Says
{
    SAY_WARN_INHALE    = 0,
    SAY_WARN_2         = 1,
    SAY_WARN_WORM      = 2,
    SAY_WARN_4         = 3
};

enum Spells
{
    SPELL_CORPSE_BREATH     = 165578,
    SPELL_NECROTIC_PITCH    = 153691,
    SPELL_FETID_SPIT        = 153681,
    SPELL_BODY_SLAM         = 154175,
    SPELL_INHALE_VISUAL     = 154868,
    SPELL_INHALE_AT         = 153999,
    SPELL_INHALE_AT_2       = 153713,
    SPELL_INHALE_PLR        = 153908,
    //Carrion Worm
    SPELL_FETID_SPIT_2      = 153496,
    SPELL_BODY_SLAM_2       = 153395
};

enum eEvents
{
    EVENT_CORPSE_BREATH     = 1,
    EVENT_NECROTIC_PITCH    = 2,
    EVENT_FETID_SPIT        = 3,
    EVENT_BODY_SLAM         = 4,
    EVENT_INHALE_1          = 5,
    EVENT_INHALE_2          = 6
};

Position const wormPos[4] =
{
    {1785.77f, -425.45f, 201.21f, 0.46f},
    {1826.28f, -473.06f, 200.74f, 3.56f},
    {1850.34f, -493.49f, 198.88f, 4.16f},
    {1808.75f, -506.52f, 201.69f, 5.63f}
};

struct boss_bonemaw : public BossAI
{
    boss_bonemaw(Creature* creature) : BossAI(creature, DATA_BONEMAW)
    {
        SetCombatMovement(false);

        for (uint8 i = 0; i < 2; i++)
            me->SummonCreature(NPC_CARRION_WORM, wormPos[i]);
    }

    bool sumAdds;

    void Reset() override
    {
        events.Reset();
        _Reset();
        summons.DespawnAll();

        me->RemoveAllAreaObjects();
        sumAdds = false;
    }

    void EnterCombat(Unit* /*who*/) override
    {
        _EnterCombat();

        events.RescheduleEvent(EVENT_FETID_SPIT, 5000);
        events.RescheduleEvent(EVENT_CORPSE_BREATH, 4000); //41:30
        events.RescheduleEvent(EVENT_NECROTIC_PITCH, 6000);
        events.RescheduleEvent(EVENT_BODY_SLAM, 14000);
        events.RescheduleEvent(EVENT_INHALE_1, 26000);
    }

    void EnterEvadeMode() override
    {
        BossAI::EnterEvadeMode();
    }

    void JustDied(Unit* /*killer*/) override
    {
        _JustDied();
    }

    void JustSummoned(Creature* summon) override
    {
        summons.Summon(summon);

        if (summon->GetEntry() == NPC_INHALE)
        {
            summon->SetReactState(REACT_PASSIVE);
            summon->SetFacingToObject(me);
            summon->CastSpell(summon, SPELL_INHALE_AT_2, false);
        }
    }

    void MoveInLineOfSight(Unit* who) override
    {
        if (who->IsPlayer())
            if (me->IsWithinDistInMap(who, 5.0f) && me->HasAura(SPELL_INHALE_AT))
                who->CastSpell(who, SPELL_INHALE_PLR, true);
    }

    void DamageTaken(Unit* /*attacker*/, uint32& /*damage*/, DamageEffectType /*dmgType*/) override
    {
        if (me->HealthBelowPct(50) && !sumAdds)
        {
            sumAdds = true;
            Talk(SAY_WARN_WORM);

            for (uint8 i = 2; i < 4; i++)
                me->SummonCreature(NPC_CARRION_WORM, wormPos[i]);
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING) || me->HasAura(SPELL_INHALE_AT))
            return;

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_CORPSE_BREATH:
                DoCast(SPELL_CORPSE_BREATH);
                events.RescheduleEvent(EVENT_CORPSE_BREATH, 28000);
                break;
            case EVENT_NECROTIC_PITCH:
                DoCast(SPELL_NECROTIC_PITCH);
                events.RescheduleEvent(EVENT_NECROTIC_PITCH, 12000);
                break;
            case EVENT_FETID_SPIT:
                if (!me->SelectNearestPlayer(10.0f))
                    if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0, true))
                        DoCast(target, SPELL_FETID_SPIT, false);

                events.RescheduleEvent(EVENT_FETID_SPIT, 1000);
                break;
            case EVENT_BODY_SLAM:
                DoCast(SPELL_BODY_SLAM);
                if (auto victim = me->getVictim())
                    me->SetFacingToObject(victim);

                events.RescheduleEvent(EVENT_BODY_SLAM, 30000);
                break;
            case EVENT_INHALE_1:
                DoCast(SPELL_INHALE_VISUAL);
                events.RescheduleEvent(EVENT_INHALE_1, 26000);
                events.RescheduleEvent(EVENT_INHALE_2, 2000);
                break;
            case EVENT_INHALE_2:
                Talk(SAY_WARN_INHALE);
                me->SummonCreature(NPC_INHALE, 1835.6f, -522.21f, 201.73f);
                DoCast(SPELL_INHALE_AT);
                break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

//76057
struct npc_bonemaw_carrion_worm : public ScriptedAI
{
    npc_bonemaw_carrion_worm(Creature* creature) : ScriptedAI(creature)
    {
        SetCombatMovement(false);
    }

    EventMap events;

    void Reset() override
    {
        events.Reset();
    }

    void EnterCombat(Unit* /*who*/) override
    {
        events.RescheduleEvent(EVENT_FETID_SPIT, 5000);
        events.RescheduleEvent(EVENT_BODY_SLAM, 14000);
    }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        DoZoneInCombat(me, 30.0f);
    }

    void UpdateAI(uint32 diff) override
    {
        events.Update(diff);

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_FETID_SPIT:
                if (!me->SelectNearestPlayer(10.0f))
                    if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0, true))
                        DoCast(target, SPELL_FETID_SPIT_2, false);

                events.RescheduleEvent(EVENT_FETID_SPIT, 2000);
                break;
            case EVENT_BODY_SLAM:
                DoCast(SPELL_BODY_SLAM_2);

                if (auto victim = me->getVictim())
                    me->SetFacingToObject(victim);

                events.RescheduleEvent(EVENT_BODY_SLAM, 16000);
                break;
            }
        }
    }
};

void AddSC_boss_bonemaw()
{
    RegisterCreatureAI(boss_bonemaw);
    RegisterCreatureAI(npc_bonemaw_carrion_worm);
}
