/*
    Dungeon : Skyreach 97 - 99
    Encounter: High Sage Viryx
*/

#include "skyreach.h"

enum Says
{
    SAY_AGGRO               = 0,
    SAY_SUMM_VEHICLE        = 1,
    SAY_WARN_LENS_FLARE     = 2,
    SAY_SUMM_ZEALOT         = 3,
    SAY_SUMM_DEFENDER       = 4,
    SAY_KILL_PLAYER         = 5,
    SAY_DEATH               = 6
};

enum Spells
{
    //Viryx
    SPELL_SOLAR_BURST     = 154396,
    SPELL_CAST_DOWN       = 153954,
    SPELL_LENS_FLARE      = 154032,
    SPELL_CALL_ADDS       = 154049,
    SPELL_CAST_DOWN_SUMM  = 153955,
    //Zealot
    SPELL_JUMP_CREATOR    = 165834,
    SPELL_CREATOR_RIDE_ME = 136522,
    SPELL_LENS_FLARE_AT   = 154044,
    //Defender
    SPELL_SHIELDING       = 154055
};

enum eEvents
{
    EVENT_SOLAR_BURST    = 1,
    EVENT_CAST_DOWN      = 2,
    EVENT_LENS_FLARE     = 3,
    EVENT_CALL_ADDS      = 4
};

Position const centrPos = {1074.27f, 1792.10f, 262.17f};

Position const linePos[6] =
{
    {1082.44f, 1800.62f, 262.0f}, //Right line
    {1167.0f, 1844.0f, 262.0f},
    {1069.32f, 1780.34f, 262.0f}, //Left line
    {1087.0f, 1695.0f, 262.0f},
    {1044.09f, 1753.11f, 262.0f},
    {1111.97f, 1848.32f, 262.0f} //Back line
};

Position const lineEndPos[2] =
{
    {1137.63f, 1820.51f, 262.17f},
    {1088.75f, 1722.93f, 262.17f}
};

struct boss_high_sage_viryx : public BossAI
{
    explicit boss_high_sage_viryx(Creature* creature) : BossAI(creature, DATA_VIRYX) {}

    void Reset() override
    {
        events.Reset();
        _Reset();

        me->RemoveAllAreaObjects();
    }

    void EnterCombat(Unit* /*who*/) override
    {
        Talk(SAY_AGGRO);
        _EnterCombat();

        events.RescheduleEvent(EVENT_SOLAR_BURST, 8000);
        events.RescheduleEvent(EVENT_CAST_DOWN, 16000);
        events.RescheduleEvent(EVENT_LENS_FLARE, 26000);
        events.RescheduleEvent(EVENT_CALL_ADDS, 32000);
    }

    void EnterEvadeMode() override
    {
        BossAI::EnterEvadeMode();
    }

    void JustDied(Unit* /*killer*/) override
    {
        Talk(SAY_DEATH);
        _JustDied();

        me->RemoveAllAreaObjects();
    }

    void SpellHitTarget(Unit* target, SpellInfo const* spell) override
    {
        if (spell->Id == SPELL_LENS_FLARE)
            if (target->IsPlayer())
                ZoneTalk(SAY_WARN_LENS_FLARE, target->GetGUID());
    }

    void KilledUnit(Unit* victim) override
    {
        if (!victim->IsPlayer())
            return;

        if (urand(0, 1))
            Talk(SAY_KILL_PLAYER);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        if (me->GetDistance(me->GetHomePosition()) > 55.0f)
        {
            EnterEvadeMode();
            return;
        }

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_SOLAR_BURST:
                if (auto victim = me->getVictim())
                    DoCast(victim, SPELL_SOLAR_BURST, false);

                events.RescheduleEvent(EVENT_SOLAR_BURST, 12000);
                break;
            case EVENT_CAST_DOWN:
                Talk(SAY_SUMM_ZEALOT);
                DoCast(SPELL_CAST_DOWN);
                events.RescheduleEvent(EVENT_CAST_DOWN, 36000);
                break;
            case EVENT_LENS_FLARE:
                DoCast(SPELL_LENS_FLARE);
                events.RescheduleEvent(EVENT_LENS_FLARE, 36000);
                break;
            case EVENT_CALL_ADDS:
                DoCast(SPELL_CALL_ADDS);
                events.RescheduleEvent(EVENT_CALL_ADDS, 60000);
                break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

//76267
struct npc_solar_zealot : public ScriptedAI
{
    explicit npc_solar_zealot(Creature* creature) : ScriptedAI(creature)
    {
        me->setFaction(16);
        me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
        me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
        me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FEAR, true);
        me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_ROOT, true);
        me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_POLYMORPH, true);
        me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_HORROR, true);
        me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_SAPPED, true);
        me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_CHARM, true);
        me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_DISORIENTED, true);
        me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_DISTRACT, true);
        me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_CONFUSE, true);
    }

    EventMap events;

    void Reset() override
    {
        events.Reset();
        me->SetReactState(REACT_PASSIVE);
    }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        events.RescheduleEvent(EVENT_1, 1000);
    }

    void MovementInform(uint32 type, uint32 id) override
    {
        if (type != POINT_MOTION_TYPE)
            return;

        if (id == 1)
        {
            me->GetVehicleKit()->RemoveAllPassengers();
            me->DespawnOrUnsummon(1000);
        }
    }

    void SpellHitTarget(Unit* target, SpellInfo const* spell) override
    {
        if (spell->Id == SPELL_CREATOR_RIDE_ME)
        {
            if (target->GetPositionZ() > 270.0f || target->GetPositionZ() < 255.0f)
            {
                me->DespawnOrUnsummon();
                return;
            }

            target->EnterVehicle(me, 0);
            events.RescheduleEvent(EVENT_2, 1000);
        }
    }

    void UpdateAI(uint32 diff) override
    {
        events.Update(diff);

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_1:
                if (Unit* summoner = me->ToTempSummon()->GetSummoner())
                    if (summoner)
                        DoCast(summoner, SPELL_JUMP_CREATOR, true);
                break;
            case EVENT_2:
                Position pos;
                float angle = centrPos.GetAngle(me);
                centrPos.SimplePosXYRelocationByAngle(pos, 90.0f, angle);
                if (centrPos.IsLinesCross(centrPos, pos, linePos[0], linePos[1])) //Right
                    me->GetMotionMaster()->MovePoint(1, lineEndPos[0]);
                else if (centrPos.IsLinesCross(centrPos, pos, linePos[2], linePos[3])) //Left
                    me->GetMotionMaster()->MovePoint(1, lineEndPos[1]);
                else if (centrPos.IsLinesCross(centrPos, pos, linePos[4], linePos[5])) //Back
                {
                    if (Unit* summoner = me->ToTempSummon()->GetSummoner())
                    {
                        if (summoner && summoner->ToPlayer())
                        {
                            me->GetVehicleKit()->RemoveAllPassengers();
                            me->Kill(summoner);
                            me->DespawnOrUnsummon();
                        }
                    }
                }
                else
                    me->GetMotionMaster()->MovePoint(1, pos.GetPositionX(), pos.GetPositionY(), me->GetPositionZ());
                break;
            }
        }
    }
};

//76083
struct npc_viryx_lens_flare : public ScriptedAI
{
    explicit npc_viryx_lens_flare(Creature* creature) : ScriptedAI(creature) {}

    void Reset() override
    {
        me->SetReactState(REACT_PASSIVE);
    }

    void IsSummonedBy(Unit* summoner) override
    {
        me->GetMotionMaster()->MoveFollow(summoner, 0.0f, 0.0f);
    }

    void UpdateAI(uint32 diff) override {}
};

//76292
struct npc_skyreach_shield_construct : public ScriptedAI
{
    explicit npc_skyreach_shield_construct(Creature* creature) : ScriptedAI(creature) {}

    EventMap events;

    void Reset() override
    {
        events.Reset();
    }

    void EnterCombat(Unit* /*who*/) override
    {
        events.RescheduleEvent(EVENT_1, 4000);
    }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        DoZoneInCombat(me, 100.0f);
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
            case EVENT_1:
                DoCast(SPELL_SHIELDING);
                events.RescheduleEvent(EVENT_1, 15000);
                break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

//153954
class spell_viryx_cast_down : public SpellScript
{
    PrepareSpellScript(spell_viryx_cast_down);

    void FilterTargets(std::list<WorldObject*>& targets)
    {
        if (targets.size() > 1)
            if (GetCaster()->getVictim())
                targets.remove(GetCaster()->getVictim());
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_viryx_cast_down::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
    }
};

//153955
class spell_viryx_cast_down_summ : public SpellScript
{
    PrepareSpellScript(spell_viryx_cast_down_summ);

    void ModDestHeight(SpellEffIndex effIndex)
    {
        PreventHitDefaultEffect(EFFECT_0);

        static Position const offset = { 0.0f, 0.0f, 12.0f, 0.0f };
        GetHitDest()->RelocateOffset(offset);
    }

    void Register() override
    {
        OnEffectLaunch += SpellEffectFn(spell_viryx_cast_down_summ::ModDestHeight, EFFECT_0, SPELL_EFFECT_SUMMON);
    }
};
void AddSC_boss_high_sage_viryx()
{
    RegisterCreatureAI(boss_high_sage_viryx);
    RegisterCreatureAI(npc_solar_zealot);
    RegisterCreatureAI(npc_viryx_lens_flare);
    RegisterCreatureAI(npc_skyreach_shield_construct);
    RegisterSpellScript(spell_viryx_cast_down);
    RegisterSpellScript(spell_viryx_cast_down_summ);
}
