/*==============
==============*/

#include "shadopan_monastery.h"

enum eSay
{
    SAY_INTRO_1     = 0,
    SAY_INTRO_2     = 1,
    SAY_MOVE_HOME   = 8,
    SAY_READY_FIGHT = 9,
    SAY_AGGRO       = 11
};

enum eSpells
{
    // Snowdrift

    SPELL_CHASE_DOWN            = 118961,
    SPELL_FIST_OF_FURY          = 106853,
    SPELL_PARRY_STANCE          = 106454,
    SPELL_QUIVERING_PALM        = 106422,
    SPELL_TORNADO_KICK          = 106434,

    SPELL_SMOKE_BOMB            = 110444,

    // Novices
    SPELL_ACHIEV_RESPECT        = 118918,

    // Flagrant Lotus
    SPELL_BALL_OF_FIRE          = 113760,
    SPELL_BALL_OF_FIRE_DAMAGE   = 106470,
    SPELL_FLYING_KICK           = 106439,

    SPELL_CALL_STAFF            = 106681,
    SPELL_RELEASE_STAFF         = 106680,

    // Flying Snow
    SPELL_WHIRLING_STEEL_FOCUS  = 106699,
    SPELL_WHIRLING_STEEL_DAMAGE = 106646,

    // Both
    SPELL_FLARE                 = 132951,
    
    SPELL_ENCOUNTER_CREDIT      = 123096
};

enum eEvents
{
    // Snowdrift
    EVENT_FIRST_EVENT           = 1,
    EVENT_INTRO                 = 2,
    EVENT_CHASE_DOWN            = 3,
    EVENT_FIST_OF_FURY          = 4,
    EVENT_PARRY_STANCE          = 5,
    EVENT_QUIVERING_PALM        = 6,
    EVENT_TORNADO_KICK          = 7,
    EVENT_FIREBALL              = 8,
    EVENT_PHASE_3               = 9,

    EVENT_DISAPPEAR             = 10,
    EVENT_DISAPPEAR_TWO         = 11,

    // Miniboss
    EVENT_BALL_OF_FIRE          = 12,
    EVENT_FLYING_KICK           = 13,
    EVENT_CALL_STAFF            = 14,
    EVENT_RELEASE_STAFF         = 15,
    
    EVENT_WHIRLING_STEEL_FOCUS  = 16,
    EVENT_WHIRLING_STEEL_CHANGE = 17,
    EVENT_WHIRLING_STEEL_STOP   = 18,
    
    EVENT_FORFEIT_JUMP          = 19
};

enum ePhases
{
    PHASE_FIRST_EVENT   = 1,
    PHASE_FIGHT_1       = 2,
    PHASE_FIGHT_2       = 3,
    PHASE_FIGHT_3       = 4
};

enum ePoints
{
    POINT_BEGIN_EVENT               = 1,
    POINT_PHASE_FIGHT               = 2,

    POINT_NOVICE_JUMP               = 3,
    POINT_NOVICE_DEFEATED           = 4,
    POINT_NOVICE_DEFEATED_SECOND    = 5,

    POINT_MINIBOSS_JUMP             = 6,
    POINT_MINIBOSS_DEFEATED         = 7
};

Position InitiateSpawnPos[5] =
{
    {3708.56f, 3039.60f, 816.28f},
    {3699.19f, 3049.62f, 816.28f},
    {3688.03f, 3055.68f, 816.28f},
    {3675.10f, 3066.98f, 816.28f},
    {3668.95f, 3070.81f, 816.28f}
};

Position MinibossSpawnPos[2] =
{
    {3683.72f, 3053.94f, 816.28f},
    {3687.77f, 3051.30f, 816.28f}
};

Position ClonePos[3] =
{
    {3683.37f, 3087.65f, 815.70f, 0.0f},
    {3719.41f, 3062.31f, 815.70f, 1.88f},
    {3718.04f, 3097.97f, 817.40f, 4.06f}
};

struct boss_master_snowdrift : public BossAI
{
    explicit boss_master_snowdrift(Creature* creature) : BossAI(creature, DATA_MASTER_SNOWDRIFT)
    {
        EncounterFinish = false;
    }

    EventMap events;

    bool introStarted;
    bool miniboss;
    bool EncounterFinish;

    uint8 phase;
    uint8 eventPhase;

    void Reset() override
    {
        events.Reset();
        _Reset();
        introStarted = false;
        miniboss = true;
        phase = PHASE_FIRST_EVENT;
        eventPhase = 0;
        if (!EncounterFinish)
        {
            me->GetMotionMaster()->Clear();
            me->GetMotionMaster()->MovePoint(POINT_BEGIN_EVENT, 3680.56f, 3045.27f, 816.20f);
        }
        me->setFaction(35);
        me->SetReactState(REACT_PASSIVE);
        SetCanSeeEvenInPassiveMode(true);
    }

    void EnterCombat(Unit* /*who*/) override
    {
        instance->SetBossState(DATA_MASTER_SNOWDRIFT, IN_PROGRESS);
        initDefaultEventsForPhase();
        Talk(SAY_AGGRO);
    }

    void initDefaultEventsForPhase()
    {
        events.CancelEventGroup(PHASE_FIGHT_1);
        events.CancelEventGroup(PHASE_FIGHT_2);
        events.CancelEventGroup(PHASE_FIGHT_3);

        switch (phase)
        {
        case PHASE_FIGHT_1:
            events.RescheduleEvent(EVENT_TORNADO_KICK, urand(7500, 12500), PHASE_FIGHT_1);
            events.RescheduleEvent(EVENT_FIST_OF_FURY, urand(5000, 10000), PHASE_FIGHT_1);
            events.RescheduleEvent(EVENT_CHASE_DOWN, urand(1000, 10000), PHASE_FIGHT_1);
            break;
        case PHASE_FIGHT_2:
            events.RescheduleEvent(EVENT_FIREBALL, urand(2500, 5000), PHASE_FIGHT_2);
            events.RescheduleEvent(EVENT_PHASE_3, urand(10000, 15000), PHASE_FIGHT_2);
            break;
        case PHASE_FIGHT_3:
            events.RescheduleEvent(EVENT_PARRY_STANCE, urand(10000, 15000), PHASE_FIGHT_3);
            events.RescheduleEvent(EVENT_QUIVERING_PALM, urand(5000, 10000), PHASE_FIGHT_3);
            break;
        }
    }

    void MoveInLineOfSight(Unit* who) override
    {
        if (who->ToPlayer() && !introStarted && !EncounterFinish)
        {
            if (who->ToPlayer()->isGameMaster())
                return;

            if (me->GetDistance(who) < 45.0f)
            {
                introStarted = true;
                Talk(SAY_INTRO_1);
                instance->SetBossState(DATA_MASTER_SNOWDRIFT, SPECIAL);
                events.RescheduleEvent(EVENT_FIRST_EVENT, 1000);
                events.RescheduleEvent(EVENT_INTRO, 5000);

                if (auto endoor = me->FindNearestGameObject(213194, 80.0f))
                    endoor->SetGoState(GO_STATE_READY);
            }
        }
    }

    void JustDied(Unit* /*killer*/) override
    {
        _JustDied();
    }

    void EnterEvadeMode() override
    {
        if (!EncounterFinish)
        {
            me->GetMotionMaster()->Clear();
            me->GetMotionMaster()->MovePoint(POINT_BEGIN_EVENT, 3680.56f, 3045.27f, 816.20f);//Events pos

            if (instance)
                instance->SetBossState(DATA_MASTER_SNOWDRIFT, FAIL);
        }
        summons.DespawnAll();
    }

    void JustReachedHome() override
    {
        if (EncounterFinish)
        {
            me->GetMap()->UpdateEncounterState(ENCOUNTER_CREDIT_CAST_SPELL, SPELL_ENCOUNTER_CREDIT, me, me);
            me->HandleEmoteCommand(EMOTE_STATE_SIT);
        }
    }

    void DoAction(int32 const action) override
    {
        if (action == ACTION_NOVICE_DONE)
        {
            std::list<Creature*> noviceList;
            GetCreatureListWithEntryInGrid(noviceList, me, NPC_NOVICE, 100.0f);

            for (std::list<Creature*>::const_iterator itr = noviceList.begin(); itr != noviceList.end(); ++itr)
            {
                if (Creature* position = instance->instance->GetCreature(instance->GetGuidData(DATA_RANDOM_SECOND_POS)))
                {
                    (*itr)->GetMotionMaster()->Clear();
                    (*itr)->GetMotionMaster()->MoveJump(position->GetPositionX(), position->GetPositionY(), position->GetPositionZ(), 20.0f, 30.0f, POINT_NOVICE_DEFEATED_SECOND);
                }
            }

            ++eventPhase;
            events.RescheduleEvent(EVENT_FIRST_EVENT, urand(1000, 2000));
        }
        else if (action == ACTION_MINIBOSS_DONE)
        {
            ++eventPhase;
            events.RescheduleEvent(EVENT_FIRST_EVENT, urand(1000, 2000));
        }
    }

    void MovementInform(uint32 uiType, uint32 id) override
    {
        if (uiType != POINT_MOTION_TYPE)
            return;

        switch (id)
        {
        case POINT_BEGIN_EVENT:
            me->SetFacingTo(me->GetAngle(3659.08f, 3015.38f));
            break;
        case POINT_PHASE_FIGHT:
            me->setFaction(14);
            me->SetReactState(REACT_AGGRESSIVE);
            Talk(SAY_READY_FIGHT);
            break;
        default:
            break;
        }
    }

    void DoEvent()
    {
        switch (eventPhase)
        {
        case 0:
        case 1:
        case 2:
            ++eventPhase;
            events.RescheduleEvent(EVENT_FIRST_EVENT, 5000);
            break;
            // There is five novices waves
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        {
            // Last wave have 4 initiates (for a total of 24)
            uint8 maxInitiateByWave = eventPhase == 7 ? 4 : 5;

            for (uint8 i = 0; i < maxInitiateByWave; ++i)
                me->SummonCreature(NPC_NOVICE, InitiateSpawnPos[i].GetPositionX(), InitiateSpawnPos[i].GetPositionY(), InitiateSpawnPos[i].GetPositionZ());

            // When last wave, we wait for all initiate to be defeated
            if (eventPhase != 7)
            {
                ++eventPhase;
                events.RescheduleEvent(EVENT_FIRST_EVENT, urand(12500, 17500));
            }
            break;
        }
        case 8:
        {
            if (miniboss)
            {
                miniboss = false;
                me->SummonCreature(NPC_FLAGRANT_LOTUS, MinibossSpawnPos[0].GetPositionX(), MinibossSpawnPos[0].GetPositionY(), MinibossSpawnPos[0].GetPositionZ());
                me->SummonCreature(NPC_FLYING_SNOW, MinibossSpawnPos[1].GetPositionX(), MinibossSpawnPos[1].GetPositionY(), MinibossSpawnPos[1].GetPositionZ());
            }
            break;
        }
        case 9:
        {
            me->GetMotionMaster()->MovePoint(POINT_PHASE_FIGHT, 3713.60f, 3091.87f, 817.31f);//Pos in DoJo
            phase = PHASE_FIGHT_1;
            Talk(SAY_MOVE_HOME);
            break;
        }
        default:
            break;
        }
    }

    void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType /*dmgType*/) override
    {
        if (damage >= me->GetHealth())
        {
            damage = 0;

            if (!EncounterFinish)
            {
                EncounterFinish = true;
                me->setFaction(35);
                me->AttackStop();
                me->RemoveAllAuras();
                instance->SetBossState(DATA_MASTER_SNOWDRIFT, DONE);
                DoCast(SPELL_ENCOUNTER_CREDIT);
                me->CombatStop();
                EnterEvadeMode();
                me->GetMotionMaster()->MoveTargetedHome();
                me->DespawnOrUnsummon(10000);
            }
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (EncounterFinish)
            return;

        events.Update(diff);

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_FIRST_EVENT:
                DoEvent();
                break;
            case EVENT_INTRO:
                Talk(SAY_INTRO_2);
                break;
            case EVENT_TORNADO_KICK:
                DoCast(SPELL_TORNADO_KICK);
                events.RescheduleEvent(EVENT_TORNADO_KICK, urand(7500, 12500));
                break;
            case EVENT_FIST_OF_FURY:
                if (auto target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                    DoCast(target, SPELL_FIST_OF_FURY, false);

                events.RescheduleEvent(EVENT_FIST_OF_FURY, urand(5000, 10000));
                break;
            case EVENT_CHASE_DOWN:
                events.RescheduleEvent(eventId, urand(1000, 10000));
                break;
                // Phase 2
            case EVENT_DISAPPEAR:
            {
                std::vector<uint8> randomIndex;
                for (int i = 0; i < 3; ++i) randomIndex.push_back(i);
                std::random_shuffle(randomIndex.begin(), randomIndex.end());

                bool isBoss = true;

                for (std::vector<uint8>::const_iterator index = randomIndex.begin(); index != randomIndex.end(); ++index)
                {
                    // The first random pos is for the boss, the two others are for his clones
                    if (isBoss)
                    {
                        me->NearTeleportTo(ClonePos[*index].GetPositionX(), ClonePos[*index].GetPositionY(), ClonePos[*index].GetPositionZ(), ClonePos[*index].GetOrientation());
                        me->SetVisible(true);
                        DoCast(me, SPELL_SMOKE_BOMB, true);
                        isBoss = false;
                    }
                    else
                        if (auto clone = me->SummonCreature(NPC_SNOWDRIFT_CLONE, ClonePos[*index].GetPositionX(), ClonePos[*index].GetPositionY(), ClonePos[*index].GetPositionZ(), ClonePos[*index].GetOrientation()))
                            clone->CastSpell(clone, SPELL_SMOKE_BOMB, true);
                }
                initDefaultEventsForPhase();
                break;
            }
            case EVENT_FIREBALL:
                if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                    DoCast(target, SPELL_BALL_OF_FIRE, false);

                events.RescheduleEvent(EVENT_FIREBALL, urand(2500, 5000));
                break;
                // Phase 3
            case EVENT_PHASE_3:
                me->SetReactState(REACT_AGGRESSIVE);
                phase = PHASE_FIGHT_3;
                initDefaultEventsForPhase();
                break;
            case EVENT_PARRY_STANCE:
                DoCast(SPELL_PARRY_STANCE);
                events.RescheduleEvent(EVENT_PARRY_STANCE, urand(10000, 15000));
                break;
            case EVENT_QUIVERING_PALM:
                if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                    DoCast(target, SPELL_QUIVERING_PALM, false);

                events.RescheduleEvent(EVENT_QUIVERING_PALM, urand(5000, 10000));
                break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

struct npc_snowdrift_novice : public ScriptedAI
{
    explicit npc_snowdrift_novice(Creature* creature) : ScriptedAI(creature)
    {
        instance = creature->GetInstanceScript();
        jumpDone = false;
        stillInFight = true;
    }

    InstanceScript* instance;
    EventMap events;
    bool jumpDone;
    bool stillInFight;
    bool eventEmote;

    void Reset() override
    {
        if (!jumpDone)
        {
            float x, y;
            GetPositionWithDistInOrientation(me, 40.0f, 4.0f, x, y);
            me->GetMotionMaster()->MoveJump(x, y, me->GetMap()->GetHeight(x, y, me->GetPositionZ()), 20, 10, POINT_NOVICE_JUMP);
            jumpDone = true;
        }
    }

    void MovementInform(uint32 uiType, uint32 id) override
    {
        if (uiType != EFFECT_MOTION_TYPE)
            return;

        switch (id)
        {
        case POINT_NOVICE_JUMP:
            if (auto target = me->SelectNearestPlayerNotGM(100.0f))
                AttackStart(target);
            break;
        case POINT_NOVICE_DEFEATED_SECOND:
            me->SetFacingTo(me->GetAngle(3659.08f, 3015.38f));
            break;
        default:
            break;
        }
    }

    void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType dmgType) override
    {
        if (damage >= me->GetHealth())
        {
            damage = 0;
            me->SetReactState(REACT_PASSIVE);
            me->AttackStop();
            me->setFaction(35);
            me->HandleEmoteCommand(EMOTE_ONESHOT_BOW);
            eventEmote = true;
            me->RemoveAllAuras();
            if (instance && stillInFight)
            {
                stillInFight = false;
                instance->SetData(DATA_DEFEATED_NOVICE, 1);
            }
            events.RescheduleEvent(EVENT_FORFEIT_JUMP, 2000);
        }
    }

    void JustReachedHome() override
    {
        if (auto target = me->FindNearestCreature(56505, 30.0f))
        {
            eventEmote = false;
            me->SetFacingToObject(target);
        }
    }

    void ReceiveEmote(Player* player, uint32 emote) override
    {
        if (emote == TEXT_EMOTE_BOW && eventEmote)
        {
            eventEmote = false;
            DoCast(player, SPELL_ACHIEV_RESPECT, true);
        }
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
            case EVENT_FORFEIT_JUMP:
                if (auto position = instance->instance->GetCreature(instance->GetGuidData(DATA_RANDOM_FIRST_POS)))
                {
                    EnterEvadeMode();
                    me->CombatStop();
                    me->SetHomePosition(position->GetPositionX(), position->GetPositionY(), position->GetPositionZ(), position->GetOrientation());
                    me->GetMotionMaster()->MoveJump(position->GetPositionX(), position->GetPositionY(), position->GetPositionZ(), 20.0f, 10.0f, POINT_NOVICE_DEFEATED);
                }
                break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

struct npc_snowdrift_miniboss : public ScriptedAI
{
    explicit npc_snowdrift_miniboss(Creature* creature) : ScriptedAI(creature)
    {
        instance = creature->GetInstanceScript();
        jumpDone = false;
        stillInFight = true;
    }

    InstanceScript* instance;
    EventMap events;
    bool jumpDone;
    bool stillInFight;
    uint8 whirlwindProgress;

    void Reset() override
    {
        events.Reset();
        whirlwindProgress = 0;

        if (!jumpDone)
        {
            float x, y;
            GetPositionWithDistInOrientation(me, 30.0f, 4.23f, x, y);
            me->GetMotionMaster()->MoveJump(x, y, me->GetMap()->GetHeight(x, y, me->GetPositionZ()), 20, 10, POINT_MINIBOSS_JUMP);
            jumpDone = true;

            if (me->GetEntry() == NPC_FLAGRANT_LOTUS)
            {
                events.RescheduleEvent(EVENT_BALL_OF_FIRE, 1000);
                events.RescheduleEvent(EVENT_FLYING_KICK, 1000);
                events.RescheduleEvent(EVENT_CALL_STAFF, 1000);
                events.RescheduleEvent(EVENT_RELEASE_STAFF, 1000);
            }
            else
                events.RescheduleEvent(EVENT_WHIRLING_STEEL_FOCUS, 1000);
        }
    }

    void MovementInform(uint32 uiType, uint32 id) override
    {
        if (uiType != POINT_MOTION_TYPE)
            return;

        switch (id)
        {
        case POINT_MINIBOSS_DEFEATED:
            me->SetFacingTo(me->GetAngle(3659.08f, 3015.38f));
            break;
        default:
            break;
        }
    }

    void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType /*dmgType*/) override
    {
        if (!stillInFight)
            return;

        if (damage >= me->GetHealth())
        {
            damage = 0;
            EnterEvadeMode();
            me->setFaction(35);
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
            stillInFight = false;

            if (instance)
                instance->SetData(DATA_DEFEATED_MINIBOSS, 1);

            if (auto position = instance->instance->GetCreature(instance->GetGuidData(DATA_RANDOM_MINIBOSS_POS)))
            {
                me->GetMotionMaster()->MovePoint(POINT_MINIBOSS_DEFEATED, position->GetPositionX(), position->GetPositionY(), position->GetPositionZ());
                me->SetHomePosition(position->GetPositionX(), position->GetPositionY(), position->GetPositionZ(), position->GetOrientation());
            }
            me->DespawnOrUnsummon();
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (!stillInFight)
            return;

        events.Update(diff);

        switch (events.ExecuteEvent())
        {
        case EVENT_BALL_OF_FIRE:
            if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                DoCast(target, SPELL_BALL_OF_FIRE, false);

            events.RescheduleEvent(EVENT_BALL_OF_FIRE, 10000);
            break;
        case EVENT_FLYING_KICK:
            if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                DoCast(target, SPELL_FLYING_KICK, false);

            events.RescheduleEvent(EVENT_FLYING_KICK, 10000);
            break;
        case EVENT_CALL_STAFF:
            //me->CastSpell(me, SPELL_CALL_STAFF, false);
            events.RescheduleEvent(EVENT_CALL_STAFF, 10000);
            break;
        case EVENT_RELEASE_STAFF:
            if (me->GetVehicleKit())
                if (me->GetVehicleKit()->GetPassenger(0))
                    me->GetVehicleKit()->GetPassenger(0)->ExitVehicle();
            events.RescheduleEvent(EVENT_CALL_STAFF, 10000);
            break;
        case EVENT_WHIRLING_STEEL_FOCUS:
            me->AddAura(SPELL_WHIRLING_STEEL_DAMAGE, me);
            me->SetReactState(REACT_PASSIVE);
            break;
        case EVENT_WHIRLING_STEEL_CHANGE:
            if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
            {
                me->AddAura(SPELL_WHIRLING_STEEL_FOCUS, target);
                me->GetMotionMaster()->Clear();
                me->GetMotionMaster()->MoveChase(target);
            }
            if (++whirlwindProgress >= 5)
                events.RescheduleEvent(EVENT_WHIRLING_STEEL_CHANGE, 2000);
            else
                events.RescheduleEvent(EVENT_WHIRLING_STEEL_STOP, 2000);
            break;
        case EVENT_WHIRLING_STEEL_STOP:
            me->SetReactState(REACT_AGGRESSIVE);
            me->GetMotionMaster()->Clear();

            if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                AttackStart(target);

            events.RescheduleEvent(EVENT_WHIRLING_STEEL_FOCUS, 10000);
            break;
        }
        DoMeleeAttackIfReady();
    }
};

struct npc_snowdrift_clone : public ScriptedAI
{
    explicit npc_snowdrift_clone(Creature* creature) : ScriptedAI(creature) {}

    uint32 fireBallTimer;

    void Reset() override
    {
        me->SetReactState(REACT_PASSIVE);
        me->setFaction(14);
        DoZoneInCombat();

        fireBallTimer = 500;
    }

    void UpdateAI(uint32 diff) override
    {
        if (fireBallTimer <= diff)
        {
            if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                DoCast(target, SPELL_BALL_OF_FIRE, true);

            fireBallTimer = 500;
        }
        else
            fireBallTimer -= diff;
    }
};

struct npc_snowdrift_fireball : public ScriptedAI
{
    explicit npc_snowdrift_fireball(Creature* creature) : ScriptedAI(creature) {}

    uint32 damageTimer;

    void Reset() override
    {
        float x, y;
        GetPositionWithDistInOrientation(me, 100.0f, me->GetOrientation(), x, y);
        me->GetMotionMaster()->MovePoint(0, x, y, me->GetPositionZ());

        me->SetReactState(REACT_PASSIVE);
        me->setFaction(14);

        damageTimer = 500;
    }

    void UpdateAI(uint32 diff) override
    {
        if (damageTimer <= diff)
        {
            if (auto target = me->SelectNearestTarget())
                if (me->GetDistance(target) <= 1.0f)
                    DoCast(target, SPELL_BALL_OF_FIRE_DAMAGE, true);

            damageTimer = 500;
        }
        else
            damageTimer -= diff;
    }
};

void AddSC_boss_master_snowdrift()
{
    RegisterCreatureAI(boss_master_snowdrift);
    RegisterCreatureAI(npc_snowdrift_novice);
    RegisterCreatureAI(npc_snowdrift_miniboss);
    //RegisterCreatureAI(npc_snowdrift_clone);
    //RegisterCreatureAI(npc_snowdrift_fireball);
}
