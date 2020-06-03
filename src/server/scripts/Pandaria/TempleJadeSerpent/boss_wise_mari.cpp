/*=============
==============*/

enum eBoss
{
    BOSS_WASE_MARI = 1
};

enum eSpells
{
    SPELL_WATER_BUBBLE              = 106062,
    SPELL_CALL_WATER                = 106526,
    SPELL_CORRUPTED_FOUTAIN         = 106518,
    SPELL_SHA_RESIDUE               = 106653,
    SPELL_HYDROLANCE_PRECAST        = 115220,
    SPELL_HYDROLANCE_DMG_BOTTOM     = 106267,
    SPELL_HYDROLANCE_VISUAL         = 106055,
    SPELL_HYDROLANCE_DMG            = 106105,
    SPELL_WASH_AWAY                 = 106331,
    SPELL_PURIFIED_WATER            = 118714,
    //Achiev: Hydrophobia
    SPELL_ACHIEV_CREDIT_DRY         = 128437
};

enum eTexts
{
    TEXT_INTRO            = 0,
    TEXT_AGGRO            = 1,
    TEXT_BOSS_EMOTE_AGGRO = 2,
    TEXT_CALL_WATER       = 3,
    TEXT_PHASE_SWITCH     = 4,
    TEXT_DEATH            = 5,
    TEXT_KILL_PLAYER      = 6
};

enum eEvents
{
    EVENT_CALL_WATER        = 1,
    EVENT_HYDROLANCE        = 2,
    EVENT_HYDROLANCE_START  = 3,
    EVENT_SWITCH_PHASE_TWO  = 4,
    EVENT_WASH_AWAY         = 5
};

enum eCreatures
{
    CREATURE_FOUTAIN_TRIGGER            = 56586,
    CREATURE_CORRUPT_DROPLET            = 62358,
    CREATURE_HYDROLANCE_BOTTOM_TRIGGER  = 56542
};

enum eTimers
{
    TIMER_CALL_WATTER           = 29000,
    TIMER_HYDROLANCE_START      = 10000,
    TIMER_HYDROLANCE            =  5500,
    TIMER_SWITCH_PHASE_TWO      = 15000,
    TIMER_WASH_AWAY             = 125
};

enum hydrolancePhase
{
    HYDROLANCE_BOTTOM   = 1,
    HYDROLANCE_LEFT     = 2,
    HYDROLANCE_RIGHT    = 3
};

static const float fountainTriggerPos[4][3] = 
{
    {1022.743f, -2544.295f, 173.7757f},
    {1023.314f, -2569.695f, 176.0339f},
    {1059.943f, -2581.648f, 176.1427f},
    {1075.231f, -2561.335f, 173.8758f}
};

static const float hydrolanceLeftTrigger[5][3] =
{
    {1061.411f, -2570.721f, 174.2403f},
    {1058.921f, -2573.487f, 174.2403f},
    {1055.910f, -2575.674f, 174.2403f},
    {1052.511f, -2577.188f, 174.2403f},
    {1048.871f, -2577.961f, 174.2403f}
};

static const float hydrolanceRightTrigger[5][3] =
{
    {1035.333f, -2573.693f, 174.2403f},
    {1032.795f, -2570.971f, 174.2403f},
    {1030.878f, -2567.781f, 174.2403f},
    {1029.667f, -2564.263f, 174.2403f},
    {1029.213f, -2560.569f, 174.2403f}
};

struct boss_wase_mari : public BossAI
{
    explicit boss_wase_mari(Creature* creature) : BossAI(creature, BOSS_WASE_MARI)
    {
        creature->AddUnitState(UNIT_STATE_ROOT | UNIT_STATE_CANNOT_TURN);
        ennemyInArea = false;
        intro = false;
    }

    bool ennemyInArea;
    bool intro;
    uint8 phase;
    uint8 foutainCount;
    ObjectGuid foutainTrigger[4];
    uint32 hydrolancePhase;

    void Reset() override
    {
        for (uint8 i = 0; i < 4; i++)
            foutainTrigger[i].Clear();

        hydrolancePhase = 0;
        foutainCount = 0;
        phase = 0;

        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_ACHIEV_CREDIT_DRY);
        me->RemoveAurasDueToSpell(SPELL_WATER_BUBBLE);

        _Reset();
    }

    void EnterEvadeMode() override
    {
        std::list<Creature*> creList;
        GetCreatureListWithEntryInGrid(creList, me, CREATURE_CORRUPT_DROPLET, 200.0f);
        GetCreatureListWithEntryInGrid(creList, me, 56511, 200.0f);
        if (!creList.empty())
            for (auto& cre : creList)
                cre->DespawnOrUnsummon();

        ScriptedAI::EnterEvadeMode();
    }

    void EnterCombat(Unit* /*who*/) override
    {
        std::list<Creature*> searcher;
        GetCreatureListWithEntryInGrid(searcher, me, CREATURE_FOUTAIN_TRIGGER, 50.0f);
        uint8 tab = 0;

        for (std::list<Creature*>::const_iterator itr = searcher.begin(); itr != searcher.end(); ++itr)
        {
            if (!(*itr))
                continue;

            (*itr)->RemoveAllAuras();

            foutainTrigger[++tab] = (*itr)->GetGUID();
        }

        searcher.clear();
        GetCreatureListWithEntryInGrid(searcher, me, CREATURE_CORRUPT_DROPLET, 50.0f);

        for (std::list<Creature*>::const_iterator itr = searcher.begin(); itr != searcher.end(); ++itr)
        {
            if (!(*itr))
                continue;

            if ((*itr)->isSummon())
                (*itr)->ForcedDespawn();
        }

        me->SetInCombatWithZone();
        DoCast(me, SPELL_WATER_BUBBLE, true);
        DoCast(SPELL_ACHIEV_CREDIT_DRY);
        Talk(TEXT_AGGRO);
        Talk(TEXT_BOSS_EMOTE_AGGRO);
        intro = true;
        phase = 1;
        hydrolancePhase = HYDROLANCE_BOTTOM;
        events.RescheduleEvent(EVENT_CALL_WATER, 8000);
        events.RescheduleEvent(EVENT_HYDROLANCE_START, TIMER_HYDROLANCE_START);

        _EnterCombat();
    }

    void KilledUnit(Unit* victim) override
    {
        if (!victim && !victim->IsPlayer())
            return;

        Talk(TEXT_KILL_PLAYER);
    }

    void JustDied(Unit* /*killer*/) override
    {
        Talk(TEXT_DEATH);
        DoCast(SPELL_PURIFIED_WATER);
        _JustDied();
        instance->instance->ApplyOnEveryPlayer([&](Player* player) { player->CastSpell(player, 121483, false); });
    }

    void UpdateAI(uint32 diff) override
    {
        if (!intro)
        {
            if (auto player = me->FindNearestPlayer(20.0f, true))
            {
                if (!player->isGameMaster())
                {
                    if (!ennemyInArea)
                    {
                        AttackStart(player);
                        ennemyInArea = true;
                    }
                }
            }
        }

        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (me->GetUInt32Value(UNIT_FIELD_TARGET))
            me->SetUInt32Value(UNIT_FIELD_TARGET, 0);

        if (me->HasUnitState(UNIT_STATE_CASTING) && phase != 2)
            return;

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_CALL_WATER:
            {
                if (phase != 1)
                    break;

                Talk(TEXT_CALL_WATER);

                if (auto trigger = me->GetCreature(*me, foutainTrigger[++foutainCount]))
                {
                    DoCast(trigger, SPELL_CALL_WATER, true);
                    trigger->AddAura(SPELL_CORRUPTED_FOUTAIN, trigger);
                }

                if (foutainCount == 4)
                {
                    phase = 2;
                    events.RescheduleEvent(EVENT_SWITCH_PHASE_TWO, TIMER_SWITCH_PHASE_TWO);
                    break;
                }

                events.RescheduleEvent(EVENT_CALL_WATER, TIMER_CALL_WATTER + rand() % 6000);
                break;
            }
            case EVENT_HYDROLANCE_START:
            {
                if (phase != 1)
                    break;

                float facing = 0.00f;
                events.RescheduleEvent(EVENT_HYDROLANCE, TIMER_HYDROLANCE);

                switch (hydrolancePhase)
                {
                case HYDROLANCE_BOTTOM:
                {
                    std::list<Creature*> trigger;
                    me->GetCreatureListWithEntryInGrid(trigger, CREATURE_HYDROLANCE_BOTTOM_TRIGGER, 50.0f);
                    for (auto& trigg : trigger)
                        trigg->CastSpell(trigg, SPELL_HYDROLANCE_PRECAST, true);

                    facing = 1.23f;
                    break;
                }
                case HYDROLANCE_RIGHT:
                    for (uint8 i = 0; i < 5; i++)
                        me->CastSpell(hydrolanceRightTrigger[i][0], hydrolanceRightTrigger[i][1], hydrolanceRightTrigger[i][2], SPELL_HYDROLANCE_PRECAST, true);

                    facing = 3.55f;
                    break;
                case HYDROLANCE_LEFT:
                    for (uint8 i = 0; i < 5; i++)
                        me->CastSpell(hydrolanceLeftTrigger[i][0], hydrolanceLeftTrigger[i][1], hydrolanceLeftTrigger[i][2], SPELL_HYDROLANCE_PRECAST, true);

                    facing = 5.25f;
                    break;
                }
                DoCast(SPELL_HYDROLANCE_VISUAL);
                me->UpdatePosition(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), facing);
                me->SetFacingTo(facing);
                break;
            }
            case EVENT_HYDROLANCE:
            {
                if (phase != 1)
                    break;

                switch (hydrolancePhase)
                {
                case HYDROLANCE_BOTTOM:
                {
                    std::list<Creature*> trigger;
                    me->GetCreatureListWithEntryInGrid(trigger, CREATURE_HYDROLANCE_BOTTOM_TRIGGER, 50.0f);
                    for (auto& trigg : trigger)
                        trigg->CastSpell(trigg->GetPositionX(), trigg->GetPositionY(), trigg->GetPositionZ(), SPELL_HYDROLANCE_DMG_BOTTOM, true);
                    break;
                }
                case HYDROLANCE_RIGHT:
                    for (uint8 i = 0; i < 5; i++)
                        me->CastSpell(hydrolanceRightTrigger[i][0], hydrolanceRightTrigger[i][1], hydrolanceRightTrigger[i][2], SPELL_HYDROLANCE_DMG, true);
                    break;
                case HYDROLANCE_LEFT:
                    for (uint8 i = 0; i < 5; i++)
                        me->CastSpell(hydrolanceLeftTrigger[i][0], hydrolanceLeftTrigger[i][1], hydrolanceLeftTrigger[i][2], SPELL_HYDROLANCE_DMG, true);
                    break;
                }

                if (hydrolancePhase == HYDROLANCE_RIGHT)
                    hydrolancePhase = HYDROLANCE_BOTTOM;
                else
                    hydrolancePhase++;

                events.RescheduleEvent(EVENT_HYDROLANCE_START, TIMER_HYDROLANCE_START);
                break;
            }
            case EVENT_SWITCH_PHASE_TWO:
                Talk(TEXT_PHASE_SWITCH);
                DoCast(106612);
                events.RescheduleEvent(EVENT_6, 4500);
                break;
            case EVENT_WASH_AWAY:
            {
                if (phase != 2)
                    break;

                float facing = me->GetOrientation();
                facing += M_PI / 48;

                if (facing > M_PI * 2)
                    facing -= M_PI * 2;

                me->SetOrientation(facing);
                me->SetFacingTo(facing);

                events.RescheduleEvent(EVENT_WASH_AWAY, TIMER_WASH_AWAY);
                break;
            }
            case EVENT_6:
            {
                if (phase != 2)
                    break;

                float facing = me->GetOrientation();
                facing += M_PI / 48;

                if (facing > M_PI * 2)
                    facing -= M_PI * 2;

                me->SetOrientation(facing);
                me->SetFacingTo(facing);
                DoCast(me, SPELL_WASH_AWAY, true);
                events.RescheduleEvent(EVENT_WASH_AWAY, TIMER_WASH_AWAY);
                break;
            }
            }
        }
    }
};

struct mob_corrupt_living_water : public ScriptedAI
{
    explicit mob_corrupt_living_water(Creature* creature) : ScriptedAI(creature) {}

    void IsSummonedBy(Unit* /*owner*/) override
    {
        ZoneTalk(0);
    }

    void JustDied(Unit* /*killer*/) override
    {
        for (uint8 i = 0; i < 3; ++i)
        {
            Position pos;
            me->GetRandomNearPosition(pos, 4.0f);

            if (auto droplet = me->SummonCreature(CREATURE_CORRUPT_DROPLET, pos))
                if (auto unit = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                    droplet->Attack(unit, true);
        }

        DoCast(me, SPELL_SHA_RESIDUE, true);
        me->DespawnOrUnsummon(23000);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

void AddSC_boss_wise_mari()
{
    RegisterCreatureAI(boss_wase_mari);
    RegisterCreatureAI(mob_corrupt_living_water);
}
