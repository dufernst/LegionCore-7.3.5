#include "throne_of_the_tides.h"

enum ScriptTexts
{
    SAY_AGGRO   = 0,
    SAY_KILL    = 1,
    SAY_DEATH   = 2,
    SAY_66      = 3,
    SAY_33      = 4
};

enum Spells
{
    SPELL_FUNGAL_SPORES         = 76001,
    SPELL_FUNGAL_SPORES_DMG     = 80564,
    SPELL_SHOCK_BLAST           = 76008,
    SPELL_SUMMON_GEYSER         = 75722,
    SPELL_GEYSER_VISUAL         = 75699,
    SPELL_GEYSER_ERRUPT         = 75700,
    SPELL_GEYSER_ERRUPT_KNOCK   = 94046,
    SPELL_WATERSPOUT            = 75683,
    SPELL_WATERSPOUT_KNOCK      = 75690,
    SPELL_WATERSPOUT_SUMMON     = 90495,
    SPELL_WATERSPOUT_VISUAL     = 90440,
    SPELL_WATERSPOUT_DMG        = 90479,
    SPELL_VISUAL_INFIGHT_AURA   = 91349,

    // honnor guard
    SPELL_ARC_SLASH             = 75907,
    SPELL_ENRAGE                = 75998,

    // tempest witch
    SPELL_CHAIN_LIGHTNING       = 75813,
    SPELL_LIGHTNING_SURGE       = 75992,
    SPELL_LIGHTNING_SURGE_DMG   = 75993,

    SPELL_ACHIEV_CREDIT         = 94042
};

enum Events
{
    EVENT_GEYSER            = 1,
    EVENT_GEYSER_ERRUPT     = 2,
    EVENT_FUNGAL_SPORES     = 3,
    EVENT_SHOCK_BLAST       = 4,
    EVENT_WATERSPOUT_END    = 5,
    EVENT_START_ATTACK      = 6,
    EVENT_ARC_SLASH         = 7,
    EVENT_LIGHTNING_SURGE   = 8,
    EVENT_CHAIN_LIGHTNING   = 9
};

enum Points
{
    POINT_CENTER_1      = 1,
    POINT_CENTER_2      = 2,
    POINT_WATERSPOUT    = 3
};

enum Adds
{
    NPC_TEMPEST_WITCH       = 44404,
    NPC_HONNOR_GUARD        = 40633,
    NPC_WATERSPOUT          = 48571,
    NPC_WATERSPOUT_H        = 49108,
    NPC_GEYSER              = 40597
};

const Position summonPos[3] =
{
    {174.41f, 802.323f, 808.368f, 0.014f},
    {200.517f, 787.687f, 808.368f, 2.056f},
    {200.558f, 817.046f, 808.368f, 4.141f}
};

const Position centerPos = {192.05f, 802.52f, 807.64f, 3.14f};

struct boss_lady_nazjar : public BossAI
{
    explicit boss_lady_nazjar(Creature* creature) : BossAI(creature, DATA_LADY_NAZJAR) {}

    uint8 uiSpawnCount;
    uint8 uiPhase;

    void Reset() override
    {
        _Reset();

        uiPhase = 0;
        uiSpawnCount = 3;
        me->SetReactState(REACT_AGGRESSIVE);
        events.Reset();
    }

    void SummonedCreatureDies(Creature* summon, Unit* /*killer*/) override
    {
        switch (summon->GetEntry())
        {
        case NPC_TEMPEST_WITCH:
        case NPC_HONNOR_GUARD:
            uiSpawnCount--;
            break;
        }
    }

    void KilledUnit(Unit* victim) override
    {
        if (!victim)
            return;

        if (victim->IsPlayer())
            Talk(SAY_KILL);
    }

    void SpellHit(Unit* /*caster*/, SpellInfo const* spell) override
    {
        if (me->GetCurrentSpell(CURRENT_GENERIC_SPELL))
            if (me->GetCurrentSpell(CURRENT_GENERIC_SPELL)->m_spellInfo->Id == SPELL_SHOCK_BLAST)
                for (uint8 i = 0; i < 3; ++i)
                    if (spell->Effects[i]->Effect == SPELL_EFFECT_INTERRUPT_CAST)
                        me->InterruptSpell(CURRENT_GENERIC_SPELL);
    }

    void JustSummoned(Creature* summon) override
    {
        summons.Summon(summon);

        switch (summon->GetEntry())
        {
        case NPC_TEMPEST_WITCH:
        case NPC_HONNOR_GUARD:
            if (me->isInCombat())
                summon->SetInCombatWithZone();
            break;
        case NPC_WATERSPOUT:
        case NPC_WATERSPOUT_H:
            if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
            {
                float x, y;
                me->GetNearPoint2D(x, y, 30.0f, me->GetAngle(target->GetPositionX(), target->GetPositionY()));
                summon->GetMotionMaster()->MovePoint(POINT_WATERSPOUT, x, y, 808.0f);
            }
            break;
        }
    }

    void EnterCombat(Unit* /*who*/) override
    {
        Talk(SAY_AGGRO);
        events.RescheduleEvent(EVENT_GEYSER, 11000);
        events.RescheduleEvent(EVENT_FUNGAL_SPORES, urand(3000, 10000));
        events.RescheduleEvent(EVENT_SHOCK_BLAST, urand(6000, 12000));
        instance->SetData(DATA_LADY_NAZJAR, IN_PROGRESS);
    }

    void JustDied(Unit* /*killer*/) override
    {
        _JustDied();
        Talk(SAY_DEATH);
    }

    void MovementInform(uint32 type, uint32 id) override
    {
        if (type == POINT_MOTION_TYPE)
        {
            if (id == POINT_CENTER_1)
            {
                Talk(SAY_66);
                SetCombatMovement(false);
                DoCast(SPELL_WATERSPOUT);
                me->SummonCreature(NPC_HONNOR_GUARD, summonPos[0]);
                me->SummonCreature(NPC_TEMPEST_WITCH, summonPos[1]);
                me->SummonCreature(NPC_TEMPEST_WITCH, summonPos[2]);
                events.RescheduleEvent(EVENT_WATERSPOUT_END, 60000);

                if (IsHeroic())
                    DoCast(me, SPELL_WATERSPOUT_SUMMON, true);
            }
            else if (id == POINT_CENTER_2)
            {
                Talk(SAY_33);
                SetCombatMovement(false);
                DoCast(SPELL_WATERSPOUT);
                me->SummonCreature(NPC_HONNOR_GUARD, summonPos[0]);
                me->SummonCreature(NPC_TEMPEST_WITCH, summonPos[1]);
                me->SummonCreature(NPC_TEMPEST_WITCH, summonPos[2]);
                events.RescheduleEvent(EVENT_WATERSPOUT_END, 60000);

                if (IsHeroic())
                    DoCast(me, SPELL_WATERSPOUT_SUMMON, true);
            }
        }
    }

    void WaterspoutEnd()
    {
        uiPhase++;
        events.CancelEvent(EVENT_WATERSPOUT_END);
        me->RemoveAurasDueToSpell(SPELL_WATERSPOUT_SUMMON);
        me->InterruptNonMeleeSpells(false);
        me->CastStop();
        SetCombatMovement(true);
        me->SetReactState(REACT_AGGRESSIVE);

        if (auto victim = me->getVictim())
            me->GetMotionMaster()->MoveChase(victim);

        events.RescheduleEvent(EVENT_GEYSER, 11000);
        events.RescheduleEvent(EVENT_FUNGAL_SPORES, urand(3000, 10000));
        events.RescheduleEvent(EVENT_SHOCK_BLAST, urand(6000, 12000));
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        if (CheckHomeDistToEvade(diff, 170.0f))
            return;

        if ((uiPhase == 1 || uiPhase == 3) && uiSpawnCount == 0)
        {
            WaterspoutEnd();
            return;
        }

        if (me->HealthBelowPct(66) && uiPhase == 0)
        {
            uiPhase = 1;
            uiSpawnCount = 3;
            me->InterruptNonMeleeSpells(false);
            me->SetReactState(REACT_PASSIVE);
            me->GetMotionMaster()->MovePoint(POINT_CENTER_1, centerPos);
            return;
        }
        if (me->HealthBelowPct(33) && uiPhase == 2)
        {
            uiPhase = 3;
            uiSpawnCount = 3;
            me->InterruptNonMeleeSpells(false);
            me->SetReactState(REACT_PASSIVE);
            me->GetMotionMaster()->MovePoint(POINT_CENTER_2, centerPos);
            return;
        }

        switch (uiPhase)
        {
        case 1:
        case 3:
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_WATERSPOUT_END:
                    WaterspoutEnd();
                    break;
                }
            }
            break;
        case 0:
        case 2:
        case 4:
            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_GEYSER:
                    if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                        DoCast(target, SPELL_SUMMON_GEYSER, false);

                    events.RescheduleEvent(EVENT_GEYSER, urand(14000, 17000));
                    break;
                case EVENT_FUNGAL_SPORES:
                    if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                        DoCast(target, SPELL_FUNGAL_SPORES, false);

                    events.RescheduleEvent(EVENT_FUNGAL_SPORES, urand(15000, 18000));
                    break;
                case EVENT_SHOCK_BLAST:
                    if (auto victim = me->getVictim())
                        DoCast(victim, SPELL_SHOCK_BLAST, false);

                    events.RescheduleEvent(EVENT_SHOCK_BLAST, urand(12000, 14000));
                    break;
                }
            }
            DoMeleeAttackIfReady();
            break;
        }
    }
};

struct npc_lady_nazjar_honnor_guard : public ScriptedAI
{
    explicit npc_lady_nazjar_honnor_guard(Creature* creature) : ScriptedAI(creature)
    {
        instance = creature->GetInstanceScript();
        me->SetReactState(REACT_PASSIVE);
    }

    EventMap events;
    InstanceScript* instance;
    bool bEnrage;

    void Reset() override
    {
        bEnrage = false;
        events.RescheduleEvent(EVENT_START_ATTACK, 2000);
    }

    void JustDied(Unit* killer) override
    {
        if (instance)
            if (killer && killer->IsUnit() && killer->GetEntry() == NPC_GEYSER)
                instance->DoUpdateAchievementCriteria(CRITERIA_TYPE_BE_SPELL_TARGET, SPELL_ACHIEV_CREDIT, 0, 0, me);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        if (me->HealthBelowPct(35) && !bEnrage)
        {
            DoCast(SPELL_ENRAGE);
            bEnrage = true;
            return;
        }

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_START_ATTACK:
                me->SetReactState(REACT_AGGRESSIVE);
                events.RescheduleEvent(EVENT_ARC_SLASH, 5000);
                break;
            case EVENT_ARC_SLASH:
                DoCast(SPELL_ARC_SLASH);
                events.RescheduleEvent(EVENT_ARC_SLASH, urand(7000, 10000));
                break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

struct npc_lady_nazjar_tempest_witch : public ScriptedAI
{
    explicit npc_lady_nazjar_tempest_witch(Creature* creature) : ScriptedAI(creature)
    {
        instance = creature->GetInstanceScript();
        me->SetReactState(REACT_PASSIVE);
    }

    EventMap events;
    InstanceScript* instance;

    void Reset() override
    {
        events.RescheduleEvent(EVENT_START_ATTACK, 2000);
    }

    void JustDied(Unit* killer) override
    {
        if (instance)
            if (killer && killer->IsUnit() && killer->GetEntry() == NPC_GEYSER)
                instance->DoUpdateAchievementCriteria(CRITERIA_TYPE_BE_SPELL_TARGET, SPELL_ACHIEV_CREDIT, 0, 0, me);
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
            case EVENT_START_ATTACK:
                me->SetReactState(REACT_AGGRESSIVE);
                events.RescheduleEvent(EVENT_LIGHTNING_SURGE, urand(5000, 7000));
                events.RescheduleEvent(EVENT_CHAIN_LIGHTNING, 2000);
                break;
            case EVENT_LIGHTNING_SURGE:
                if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                    DoCast(target, SPELL_LIGHTNING_SURGE, false);

                events.RescheduleEvent(EVENT_LIGHTNING_SURGE, urand(10000, 15000));
                break;
            case EVENT_CHAIN_LIGHTNING:
                if (auto victim = me->getVictim())
                    DoCast(victim, SPELL_CHAIN_LIGHTNING, false);

                events.RescheduleEvent(EVENT_CHAIN_LIGHTNING, 2000);
                break;
            }
        }
    }
};

struct npc_lady_nazjar_waterspout : public ScriptedAI
{
    explicit npc_lady_nazjar_waterspout(Creature* creature) : ScriptedAI(creature)
    {
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        me->SetReactState(REACT_PASSIVE);
        me->SetSpeed(MOVE_RUN, 0.3f);
        DoCast(me, SPELL_WATERSPOUT_VISUAL, true);
        bHit = false;
    }

    bool bHit;

    void MovementInform(uint32 type, uint32 id) override
    {
        if (type == POINT_MOTION_TYPE)
        {
            if (id == POINT_WATERSPOUT)
                me->DespawnOrUnsummon();
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (bHit)
            return;

        if (auto target = me->SelectNearestTarget(2.0f))
        {
            if (target->IsPlayer())
                return;

            bHit = true;
            target->CastSpell(target, SPELL_WATERSPOUT_DMG, true);
        }
    }
};

struct npc_lady_nazjar_geyser : public ScriptedAI
{
    explicit npc_lady_nazjar_geyser(Creature* creature) : ScriptedAI(creature)
    {
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
        me->SetReactState(REACT_PASSIVE);
        bErrupt = false;
    }

    uint32 uiErruptTimer;
    bool bErrupt;

    void IsSummonedBy(Unit* /*owner*/) override
    {
        uiErruptTimer = 5000;
        DoCast(me, SPELL_GEYSER_VISUAL, true);
    }

    void UpdateAI(uint32 diff) override
    {
        if (uiErruptTimer <= diff)
        {
            if (!bErrupt)
            {
                bErrupt = true;
                me->RemoveAurasDueToSpell(SPELL_GEYSER_VISUAL);
                DoCast(SPELL_GEYSER_ERRUPT);
            }
        }
        else
            uiErruptTimer -= diff;
    }
};

void AddSC_boss_lady_nazjar()
{
    RegisterCreatureAI(boss_lady_nazjar);
    RegisterCreatureAI(npc_lady_nazjar_honnor_guard);
    RegisterCreatureAI(npc_lady_nazjar_tempest_witch);
    RegisterCreatureAI(npc_lady_nazjar_waterspout);
    RegisterCreatureAI(npc_lady_nazjar_geyser);
}