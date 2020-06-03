/*
    Dungeon : Black Rook Hold Dungeon 100-110
    Encounter: Smashspite the Hateful
    Normal: 100%, Heroic: 100%, Mythic: 100%
*/

#include "black_rook_hold_dungeon.h"

enum Says
{
    SAY_AGGRO                   = 0,
    SAY_HATEFUL_GAZE            = 1,
    SAY_WARN_HATEFUL_GAZE       = 2,
    SAY_HATEFUL_GAZE_CHARGE     = 3,
    SAY_EARTHSHAKING_STOMP      = 4,
    SAY_BRUTAL_HAYMAKER         = 5,
    SAY_DEATH                   = 6,
};

enum Spells
{
    SPELL_ZERO_REGEN            = 72242,
    SPELL_BRUTALITY_PROC        = 198114,
    SPELL_EARTHSHAKING_STOMP    = 198073,
    SPELL_BRUTAL_HAYMAKER       = 198245,

    //Heroic
    SPELL_HATEFUL_GAZE          = 198079,
    SPELL_HATEFUL_CHARGE        = 198250,
    SPELL_HATEFUL_CHARGE_DMG    = 198080,
    SPELL_HATEFUL_CHARGE_DEBUF  = 224188,

    SPELL_FEL_VOMIT             = 198446,
    SPELL_FEL_VOMIT_DMG         = 198499,

    //Other
    SPELL_SIC_BATS              = 203163,
};

enum eEvents
{
    EVENT_EARTHSHAKING_STOMP    = 1,
    EVENT_FEL_ATTACK            = 2,
    EVENT_HATEFUL_GAZE          = 3,
};

Position const felPos[10] =
{
    {3377.12f, 7379.05f, 260.46f}, //100759
    {3405.04f, 7360.28f, 250.10f},
    {3410.22f, 7311.27f, 250.10f},
    {3392.56f, 7317.72f, 260.46f},
    {3379.99f, 7295.96f, 268.62f},
    {3388.76f, 7284.25f, 237.00f},
    {3257.77f, 7188.85f, 255.80f},
    {3352.13f, 7259.52f, 251.29f},
    {3206.34f, 7261.78f, 255.80f},
    {3267.14f, 7182.61f, 244.89f}
};

Position const centrPos = {3269.27f, 7289.28f, 231.47f};

//98949
struct boss_smashspite_the_hateful : public BossAI
{
    boss_smashspite_the_hateful(Creature* creature) : BossAI(creature, DATA_SMASHSPITE) {}

    ObjectGuid felbatContainer[10];
    std::list<uint32> felTimer;

    void Reset() override
    {
        _Reset();
        DoCast(me, SPELL_ZERO_REGEN, true);
        DoCast(me, SPELL_BRUTALITY_PROC, true);
        me->SetPower(POWER_ENERGY, 0);
        me->SetReactState(REACT_AGGRESSIVE);

        for (uint8 i = 0; i < 10; ++i)
        {
            if (auto summon = me->SummonCreature(NPC_FEL_BAT, felPos[i]))
                felbatContainer[i] = summon->GetGUID();
        }
    }

    void EnterCombat(Unit* /*who*/) override
    {
        _EnterCombat();
        Talk(SAY_AGGRO);
        felTimer = {25000, 18000, 14000, 8000};
        events.RescheduleEvent(EVENT_EARTHSHAKING_STOMP, 12000);
        events.RescheduleEvent(EVENT_FEL_ATTACK, 25000);
        if (GetDifficultyID() != DIFFICULTY_LFR && GetDifficultyID() != DIFFICULTY_NORMAL)
            events.RescheduleEvent(EVENT_HATEFUL_GAZE, 6000);
    }

    void JustDied(Unit* /*killer*/) override
    {
        _JustDied();
        Talk(SAY_DEATH);
        DoCast(me, 205271, true);

        instance->instance->SummonCreature(102094, Position(3191.87f, 7364.25f, 223.95f, 5.36f));
        instance->instance->SummonCreature(102095, Position(3196.1f, 7367.85f, 223.96f, 5.38f));
    }

    void OnRemoveAuraTarget(Unit* target, uint32 spellId, AuraRemoveMode mode) override
    {
        if (!me->isInCombat())
            return;

        if (spellId == SPELL_HATEFUL_GAZE)
        {
            Talk(SAY_HATEFUL_GAZE_CHARGE);
            me->SetFacingTo(target);
            me->CastSpell(target, SPELL_HATEFUL_CHARGE_DMG, true);
            me->GetMotionMaster()->MoveCharge(target->GetPosition(), 30.0f, SPELL_HATEFUL_CHARGE);
        }
    }

    void SpellHitTarget(Unit* target, const SpellInfo* spell) override
    {
        if (spell->Id == SPELL_HATEFUL_CHARGE_DMG)
        {
            DoCast(target, SPELL_HATEFUL_CHARGE_DEBUF, true);
            me->StopMoving();
            me->AttackStop();
            me->SetReactState(REACT_AGGRESSIVE, 1000);
        }
    }

    void MovementInform(uint32 type, uint32 id) override
    {
        if (type != POINT_MOTION_TYPE)
            return;

        if (id == SPELL_HATEFUL_CHARGE)
            me->SetReactState(REACT_AGGRESSIVE, 1000);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        if (me->GetDistance(centrPos) >= 30.0f)
        {
            EnterEvadeMode();
            return;
        }

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_EARTHSHAKING_STOMP:
                    Talk(SAY_EARTHSHAKING_STOMP);
                    DoCast(SPELL_EARTHSHAKING_STOMP);
                    events.RescheduleEvent(EVENT_EARTHSHAKING_STOMP, 17000);
                    break;
                case EVENT_FEL_ATTACK:
                    if (auto fel = me->GetCreature(*me, felbatContainer[urand(0, 9)]))
                        fel->AI()->DoAction(true);
                    if (GetDifficultyID() == DIFFICULTY_MYTHIC_DUNGEON || GetDifficultyID() == DIFFICULTY_MYTHIC_KEYSTONE)
                    {
                        if (felTimer.size() > 1)
                            felTimer.pop_front();
                        events.RescheduleEvent(EVENT_FEL_ATTACK, felTimer.front());
                    }
                    else
                        events.RescheduleEvent(EVENT_FEL_ATTACK, 25000);
                    break;
                case EVENT_HATEFUL_GAZE:
                    if (auto target = SelectTarget(SELECT_TARGET_FARTHEST, 0, 100.0f, true))
                    {
                        Talk(SAY_HATEFUL_GAZE);
                        Talk(SAY_WARN_HATEFUL_GAZE, target->GetGUID());
                        me->StopMoving();
                        me->StopAttack();
                        me->CastSpellDelay(target, SPELL_HATEFUL_GAZE, false, 200);
                        events.RecalcEventTimer(EVENT_EARTHSHAKING_STOMP, 10000);
                    }
                    events.RescheduleEvent(EVENT_HATEFUL_GAZE, 25000);
                    break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

//100759
struct npc_smashspite_fel_bat : public ScriptedAI
{
    npc_smashspite_fel_bat(Creature* creature) : ScriptedAI(creature) 
    {
        me->SetReactState(REACT_PASSIVE);
    }

    void Reset() override {}

    void DoAction(int32 const action) override
    {
        me->CastSpell(me, SPELL_FEL_VOMIT);
    }

    void OnRemoveAuraTarget(Unit* target, uint32 spellId, AuraRemoveMode mode) override
    {
        if (spellId == SPELL_FEL_VOMIT)
        {
            auto owner = me->GetAnyOwner();
            if (!owner || !owner->isInCombat())
                return;

            Position pos;
            target->GetNearPosition(pos, abs(35.0f - target->GetExactDist2d(me)), target->GetRelativeAngle(me));
            float angle = pos.GetRelativeAngle(target);
            uint32 delay = 0;

            for (uint8 i = 0; i < 25; ++i)
            {
                pos.SimplePosXYRelocationByAngle(pos, 3.0f, angle);
                pos.m_positionZ = 231.5f;
                me->CastSpellDelay(pos, SPELL_FEL_VOMIT_DMG, true, delay, nullptr, nullptr, owner->GetGUID());
                delay += 200;
            }
        }
    }

    void UpdateAI(uint32 diff) override {}
};

//102781
struct npc_brh_fel_bat : public ScriptedAI
{
    npc_brh_fel_bat(Creature* creature) : ScriptedAI(creature) {}

    ObjectGuid targetGUID;
    bool wpRun = false;
    uint32 despawnTimer = 0;
    uint32 findTargetTimer = 0;

    void Reset() override
    {
        me->SetReactState(REACT_AGGRESSIVE);
        targetGUID.Clear();

        if (me->isSummon() && !wpRun)
        {
            me->GetMotionMaster()->MovePath(10278100, false, frand(-3.0f, 3.0f), frand(-3.0f, 3.0f));
            despawnTimer = 60000;
        }
    }

    void LastWPReached() override
    {
        despawnTimer = 10000;
        me->AddDelayedEvent(200, [this] () -> void { me->GetMotionMaster()->MoveRandom(10.0f); });
    }

    void SpellHit(Unit* caster, SpellInfo const* spell) override
    {
        if (spell->Id == SPELL_SIC_BATS && targetGUID.IsEmpty())
        {
            me->StopAttack(true);
            findTargetTimer = 200;
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (despawnTimer)
        {
            if (despawnTimer <= diff)
            {
                despawnTimer = 0;

                if (!me->isInCombat())
                    me->DespawnOrUnsummon();
                else
                    despawnTimer = 5000;
            }
            else
                despawnTimer -= diff;
        }

        if (findTargetTimer)
        {
            if (findTargetTimer <= diff)
            {
                findTargetTimer = 0;
                auto target = Unit::GetUnit(*me, targetGUID);

                if (target && target->HasAura(SPELL_SIC_BATS))
                    findTargetTimer = 1000;
                else if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true, SPELL_SIC_BATS))
                {
                    targetGUID = target->GetGUID();
                    me->AddThreat(target, 10000.0f);
                    AttackStart(target);
                }
                else
                {
                    targetGUID.Clear();
                    me->SetReactState(REACT_AGGRESSIVE);
                }
            }
            else
                findTargetTimer -= diff;
        }

        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

//198114
class spell_smashpite_brutality_proc : public AuraScript
{
    PrepareAuraScript(spell_smashpite_brutality_proc);

    void OnProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        auto caster = GetCaster()->ToCreature();
        if (!caster || caster->HasUnitState(UNIT_STATE_CASTING))
            return;

        uint8 powerCount = caster->GetPower(POWER_ENERGY);

        if (powerCount < 100)
        {
            if (caster->GetMap()->GetDifficultyID() == DIFFICULTY_MYTHIC_DUNGEON || caster->GetMap()->GetDifficultyID() == DIFFICULTY_MYTHIC_KEYSTONE)
                caster->SetPower(POWER_ENERGY, powerCount + 10);
            else
                caster->SetPower(POWER_ENERGY, powerCount + urand(6, 9));
        }
        else if (caster->GetReactState() == REACT_AGGRESSIVE && caster->getVictim())
        {
            caster->AI()->Talk(SAY_BRUTAL_HAYMAKER);
            caster->CastSpell(caster->getVictim(), SPELL_BRUTAL_HAYMAKER);
        }
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_smashpite_brutality_proc::OnProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

void AddSC_boss_smashspite_the_hateful()
{
    RegisterCreatureAI(boss_smashspite_the_hateful);
    RegisterCreatureAI(npc_smashspite_fel_bat);
    RegisterCreatureAI(npc_brh_fel_bat);
    RegisterAuraScript(spell_smashpite_brutality_proc);
}