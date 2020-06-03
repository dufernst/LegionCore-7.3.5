/*==============
==============*/

#define TYPE_LIU_FLAMEHEART_STATUS 6
#define TYPE_IS_WIPE 7

enum eBosses
{
    BOSS_LIU_FLAMEHEART,
    BOSS_YU_LON,
    BOSS_TRIGGER
};

enum eSpells
{
    //LIU FLAMEHEART Event:
    SPELL_POSSESSED_BY_SHA                  = 110164, //On Spawn
    SPELL_DUST_VISUAL                       = 110518, //On Spawn
    SPELL_SERPENT_DANCE_TRIGGER             = 106878,
    SPELL_SERPENT_STRIKE                    = 106823, //2 24 24 - 5 25 38 419912664 419923538 every 10 seconds
    SPELL_SERPENT_WAVE_SUMMON_CONTROLLER    = 106982, // 1 or 2 seconds after serpent strike
    SPELL_SERPENT_KICK                      = 106856, // 419913710 419924520, every 10 seconds
    SPELL_JADE_ESSENCE                      = 106797, //AddAura on phase 2
    SPELL_JADE_SERPENT_DANCE_TRIGGER        = 106882,
    SPELL_JADE_SERPENT_STRIKE               = 106841,
    SPELL_JADE_SERPENT_WAVE_SUMMON_CONTROLLER=106995,
    SPELL_JADE_SERPENT_KICK                 = 106864,
    SPELL_DEATH_SIPHON                      = 116783,
    SPELL_SUMMON_JADE_SERPENT               = 106895,
    SPELL_JADE_SOUL                         = 106909,

    SPELL_JADE_SERPENT_HEALTH               = 106924,
    SPELL_SHARED_HEALTH                     = 114711,
    SPELL_TRANSFORM_VISUAL                  = 74620, //When the dragon is dead, cast this and remove the possess aura.
    SPELL_JADE_FIRE                         = 107045,
    SPELL_JADE_FIRE_MISSILE                 = 107098,
    SPELL_JADE_FIRE_SUMMON                  = 107103,
    SPELL_CLEANSING_BREATH                  = 132387,

    SPELL_JADE_SERPENT_WAVE_DMG             = 118540,

    SPELL_SERPENT_WAVE_SUMMON               = 118551,
    SPELL_SERPENT_WAVE_SUMMON_N             = 118549,
    SPELL_SERPENT_WAVE_SUMMON_E             = 106930,
    SPELL_SERPENT_WAVE_SUMMON_S             = 106928,
    SPELL_SERPENT_WAVE_SUMMON_W             = 106931,

    SPELL_JADE_SERPENT_WAVE_VISUAL          = 107002,
    SPELL_SERPENT_WAVE_VISUAL               = 106939,
    SPELL_SERPENT_WAVE_PERIODIC             = 106959,
    SPELL_JADE_SERPENT_WAVE_PERIODIC        = 107054,
    SPELL_JADE_SERPENT_WAVE_STALKER_PERIODIC= 106879,
    SPELL_JADE_SERPENT_WAVE                 = 119508,
    SPELL_SERPENT_WAVE                      = 106938,

    SPELL_JADE_FIRE_PERIODIC                = 107108
};

enum eStatus
{
    PHASE_1,
    PHASE_2,
    PHASE_3
};

enum eEvents
{
    EVENT_SERPENT_STRIKE        = 1,
    EVENT_SERPENT_KICK          = 2,
    EVENT_SERPENT_WAVE          = 3,

    EVENT_JADE_SERPENT_STRIKE   = 4,
    EVENT_JADE_SERPENT_KICK     = 5,
    EVENT_JADE_SERPENT_WAVE     = 6,

    EVENT_SUMMON_YULON          = 7,
    EVENT_JADE_FIRE             = 8,

    EVENT_AURA_JADE             = 9,
    EVENT_LIU_FALL              = 10
};

enum eTexts
{
    TALK_AGGRO_01,
    TALK_DEATH_01,
    TALK_EVENT_01,
    TALK_EVENT_02,
    TALK_INTRO_01,
    TALK_KILL_01,
    TALK_KILL_02
};

enum eCreatures
{
    CREATURE_TRIGGER_WAVE = 56789
};

struct boss_liu_flameheart : public BossAI
{
    explicit boss_liu_flameheart(Creature* creature) : BossAI(creature, BOSS_LIU_FLAMEHEART)
    {
        DoCast(SPELL_POSSESSED_BY_SHA);
        status = PHASE_1;
        DoCast(SPELL_DUST_VISUAL);
        wipe_timer = 2000;
        Talk(TALK_INTRO_01);
    }

    eStatus status;
    uint32 wipe_timer;
    bool pct70;
    bool pct30;

    void Reset() override
    {
        pct70 = false;
        pct30 = false;
        status = PHASE_1;
        _Reset();
    }

    void IsSummonedBy(Unit* /*owner*/) override
    {
        std::list<Creature*> creList;
        GetCreatureListWithEntryInGrid(creList, me, 62171, 60.0f);
        GetCreatureListWithEntryInGrid(creList, me, 58319, 60.0f);
        GetCreatureListWithEntryInGrid(creList, me, 57109, 60.0f);

        if (!creList.empty())
            for (auto& cre : creList)
                cre->DespawnOrUnsummon();
    }

    void DoAction(int32 const action) override
    {
        if (action == 0)
        {
            me->setFaction(35);
            me->getThreatManager().resetAllAggro();
            me->SetReactState(REACT_PASSIVE);
        }
    }

    void KilledUnit(Unit* u) override
    {
        if (!u && !u->IsPlayer())
            return;

        if (urand(0, 1))
            Talk(TALK_KILL_01);
        else
            Talk(TALK_KILL_02);
    }

    void EnterCombat(Unit* /*unit*/) override
    {
        Talk(TALK_AGGRO_01);
        events.RescheduleEvent(EVENT_SERPENT_STRIKE, 5000);
    }

    void EnterEvadeMode() override
    {
        me->ClearUnitState(UNIT_STATE_ROOT);
        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        me->ApplySpellImmune(SPELL_JADE_SOUL, IMMUNITY_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, false);
        me->ApplySpellImmune(SPELL_JADE_SOUL, IMMUNITY_DAMAGE, SPELL_SCHOOL_MASK_MAGIC, false);
        me->SetReactState(REACT_AGGRESSIVE);
        ScriptedAI::EnterEvadeMode();
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (me->HealthBelowPct(70) && status == PHASE_1 && !pct70)
        {
            pct70 = true;
            me->AddAura(SPELL_JADE_ESSENCE, me);
            events.Reset();
            events.RescheduleEvent(EVENT_JADE_SERPENT_STRIKE, 5000);
            events.RescheduleEvent(EVENT_JADE_SERPENT_KICK, 10000);
            Talk(TALK_EVENT_01);
            status = PHASE_2;
        }

        if (me->HealthBelowPct(30) && status == PHASE_2 && !pct30)
        {
            pct30 = true;
            events.Reset();
            events.RescheduleEvent(EVENT_SUMMON_YULON, 500);
            Talk(TALK_EVENT_02);
            status = PHASE_3;
        }

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_SERPENT_STRIKE:
            {
                if (auto victim = me->getVictim())
                    DoCast(victim, SPELL_SERPENT_STRIKE, false);

                instance->instance->ApplyOnEveryPlayer([&](Player* player)
                {
                    if (player->GetDistance2d(me) < 10.f)
                        player->KnockbackFrom(me->GetPositionX(), me->GetPositionY(), 10, 10);
                });

                events.RescheduleEvent(EVENT_SERPENT_STRIKE, 10000);
                events.RescheduleEvent(EVENT_SERPENT_WAVE, 4000);
            }
            break;
            case EVENT_SERPENT_KICK:
                if (auto victim = me->getVictim())
                    DoCast(victim, SPELL_SERPENT_KICK, false);

                events.RescheduleEvent(EVENT_SERPENT_KICK, 10000);
                break;
            case EVENT_SERPENT_WAVE:
            {
                if (auto sum = me->SummonCreature(CREATURE_TRIGGER_WAVE, 932.775f, -2548.743f, 179.821f, 1.254f))
                {
                    sum->SetDisplayId(11686);
                    sum->CastSpell(sum, SPELL_SERPENT_WAVE_VISUAL, false);
                    sum->CastSpell(sum, SPELL_SERPENT_WAVE_PERIODIC, false);
                    sum->GetMotionMaster()->MovePoint(0, 939.796f, -2530.586f, 179.941f);
                    sum->ForcedDespawn(3200);
                }

                if (auto sum = me->SummonCreature(CREATURE_TRIGGER_WAVE, 940.014f, -2564.114f, 179.821f, 5.978f))
                {
                    sum->SetDisplayId(11686);
                    sum->CastSpell(sum, SPELL_SERPENT_WAVE_VISUAL, false);
                    sum->CastSpell(sum, SPELL_SERPENT_WAVE_PERIODIC, false);
                    sum->GetMotionMaster()->MovePoint(0, 957.711f, -2570.030f, 179.941f);
                    sum->ForcedDespawn(3200);
                }

                if (auto sum = me->SummonCreature(CREATURE_TRIGGER_WAVE, 925.971f, -2572.423f, 179.821f, 4.395f))
                {
                    sum->SetDisplayId(11686);
                    sum->CastSpell(sum, SPELL_SERPENT_WAVE_VISUAL, false);
                    sum->CastSpell(sum, SPELL_SERPENT_WAVE_PERIODIC, false);
                    sum->GetMotionMaster()->MovePoint(0, 919.606f, -2591.245f, 179.941f);
                    sum->ForcedDespawn(3200);
                }

                if (auto sum = me->SummonCreature(CREATURE_TRIGGER_WAVE, 918.923f, -2557.356f, 179.821f, 2.821f))
                {
                    sum->SetDisplayId(11686);
                    sum->CastSpell(sum, SPELL_SERPENT_WAVE_VISUAL, false);
                    sum->CastSpell(sum, SPELL_SERPENT_WAVE_PERIODIC, false);
                    sum->GetMotionMaster()->MovePoint(0, 901.839f, -2551.843f, 179.941f);
                    sum->ForcedDespawn(3200);
                }
                break;
            }
            case EVENT_JADE_SERPENT_STRIKE:
            {
                if (auto victim = me->getVictim())
                    DoCast(victim, SPELL_JADE_SERPENT_STRIKE, false);

                instance->instance->ApplyOnEveryPlayer([&](Player* player)
                {
                    if (player->GetDistance2d(me) < 10.f)
                        player->KnockbackFrom(me->GetPositionX(), me->GetPositionY(), 10, 10);
                });

                events.RescheduleEvent(EVENT_JADE_SERPENT_STRIKE, 10000);
                events.RescheduleEvent(EVENT_JADE_SERPENT_WAVE, 4000);
                break;
            }
            case EVENT_JADE_SERPENT_KICK:
                if (auto victim = me->getVictim())
                    DoCast(victim, SPELL_JADE_SERPENT_KICK, false);

                events.RescheduleEvent(EVENT_JADE_SERPENT_KICK, 10000);
                break;
            case EVENT_JADE_SERPENT_WAVE:
            {
                if (auto sum = me->SummonCreature(CREATURE_TRIGGER_WAVE, 932.775f, -2548.743f, 179.821f, 1.254f))
                {
                    sum->SetDisplayId(11686);
                    sum->CastSpell(sum, SPELL_JADE_SERPENT_WAVE_VISUAL, false);
                    sum->CastSpell(sum, SPELL_SERPENT_WAVE_PERIODIC, false);
                    sum->GetMotionMaster()->MovePoint(0, 939.796f, -2530.586f, 179.941f);
                    sum->ForcedDespawn(3200);
                }

                if (auto sum = me->SummonCreature(CREATURE_TRIGGER_WAVE, 940.014f, -2564.114f, 179.821f, 5.978f))
                {
                    sum->SetDisplayId(11686);
                    sum->CastSpell(sum, SPELL_JADE_SERPENT_WAVE_VISUAL, false);
                    sum->CastSpell(sum, SPELL_SERPENT_WAVE_PERIODIC, false);
                    sum->GetMotionMaster()->MovePoint(0, 957.711f, -2570.030f, 179.941f);
                    sum->ForcedDespawn(3200);
                }

                if (auto sum = me->SummonCreature(CREATURE_TRIGGER_WAVE, 925.971f, -2572.423f, 179.821f, 4.395f))
                {
                    sum->SetDisplayId(11686);
                    sum->CastSpell(sum, SPELL_JADE_SERPENT_WAVE_VISUAL, false);
                    sum->CastSpell(sum, SPELL_SERPENT_WAVE_PERIODIC, false);
                    sum->GetMotionMaster()->MovePoint(0, 919.606f, -2591.245f, 179.941f);
                    sum->ForcedDespawn(3200);
                }

                if (auto sum = me->SummonCreature(CREATURE_TRIGGER_WAVE, 918.923f, -2557.356f, 179.821f, 2.821f))
                {
                    sum->SetDisplayId(11686);
                    sum->CastSpell(sum, SPELL_JADE_SERPENT_WAVE_VISUAL, false);
                    sum->CastSpell(sum, SPELL_SERPENT_WAVE_PERIODIC, false);
                    sum->GetMotionMaster()->MovePoint(0, 901.839f, -2551.843f, 179.941f);
                    sum->ForcedDespawn(3200);
                }
                break;
            }
            case EVENT_SUMMON_YULON:
                DoCast(SPELL_SUMMON_JADE_SERPENT);
                DoCast(SPELL_JADE_SOUL);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                me->ApplySpellImmune(SPELL_JADE_SOUL, IMMUNITY_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, true);
                me->ApplySpellImmune(SPELL_JADE_SOUL, IMMUNITY_DAMAGE, SPELL_SCHOOL_MASK_MAGIC, true);
                me->SetReactState(REACT_PASSIVE);
                me->AddUnitState(UNIT_STATE_ROOT);
                events.RescheduleEvent(EVENT_AURA_JADE, 3000);
                break;
            case EVENT_AURA_JADE:
                DoCast(SPELL_JADE_SOUL);
                events.RescheduleEvent(EVENT_AURA_JADE, 2500);
                break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

struct boss_yu_lon : public BossAI
{
    explicit boss_yu_lon(Creature* creature) : BossAI(creature, BOSS_YU_LON) {}

    void EnterCombat(Unit* /*unit*/) override
    {
        events.RescheduleEvent(EVENT_JADE_FIRE, 100);
    }

    void JustDied(Unit* /*died*/) override
    {
        DoCast(SPELL_CLEANSING_BREATH);
    }

    void SpellHitTarget(Unit* target, SpellInfo const* spell) override
    {
        if (spell->Id == SPELL_JADE_FIRE)
            DoCast(target, SPELL_JADE_FIRE_MISSILE, true);
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
            case EVENT_JADE_FIRE:
                DoCast(SPELL_JADE_FIRE);
                events.RescheduleEvent(EVENT_JADE_FIRE, 2000);
                break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

struct mob_trigger_liu_flameheart : public ScriptedAI
{
    explicit mob_trigger_liu_flameheart(Creature* creature) : ScriptedAI(creature)
    {
        if (me->GetInstanceScript() && me->GetInstanceScript()->GetData(TYPE_LIU_FLAMEHEART_STATUS))
            timer = 500;
        else
            timer = 0;
    }

    uint32 timer;

    void UpdateAI(uint32 diff) override
    {
        if (timer <= diff)
        {
            if (me->GetInstanceScript() && me->GetInstanceScript()->GetData(TYPE_LIU_FLAMEHEART_STATUS))
            {
                DoCast(107103);
                timer = 1000;
            }
        }
        else
            timer -= diff;
    }
};

struct mob_minion_of_doubt : public ScriptedAI
{
    explicit mob_minion_of_doubt(Creature* creature) : ScriptedAI(creature) {}

    EventMap events;

    void EnterCombat(Unit* /*unit*/) override
    {
        events.RescheduleEvent(EVENT_1, 2000);
        events.RescheduleEvent(EVENT_2, 4000);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        events.Update(diff);

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_1:
                if (auto victim = me->getVictim())
                    DoCast(victim, 110099, false);

                events.RescheduleEvent(EVENT_1, 5000);
                break;
            case EVENT_2:
                if (auto victim = me->getVictim())
                    DoCast(victim, 110125, false);

                events.RescheduleEvent(EVENT_2, 5000);
                break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

struct mob_lesser_sha : public ScriptedAI
{
    explicit mob_lesser_sha(Creature* creature) : ScriptedAI(creature) {}

    EventMap events;

    void EnterCombat(Unit* /*unit*/) override
    {
        events.RescheduleEvent(EVENT_1, 2000);
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
            case EVENT_1:
                if (auto victim = me->getVictim())
                    DoCast(victim, 122527, false);

                events.RescheduleEvent(EVENT_1, 5000);
                break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

void AddSC_boss_liu_flameheat()
{
    RegisterCreatureAI(boss_liu_flameheart);
    RegisterCreatureAI(boss_yu_lon);
    RegisterCreatureAI(mob_trigger_liu_flameheart);
    RegisterCreatureAI(mob_minion_of_doubt);
    RegisterCreatureAI(mob_lesser_sha);
}