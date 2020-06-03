/*===============
================*/

#include "mogu_shan_palace.h"

enum eBosses
{
    BOSS_MING_THE_CUNNING,
    BOSS_KUAI_THE_BRUTE,
    BOSS_HAIYAN_THE_UNSTOPPABLE
};

enum eEvents
{
    EVENT_TALK_0    = 1,
    EVENT_TALK_1,
    EVENT_JUMP_XIAN,
    EVENT_DISAPPEAR,

    EVENT_LIGHTNING_BOLT,
    EVENT_WHIRLING_DERVISH,
    EVENT_MAGNETIC_FIELD,
    EVENT_BOSS_RETIRE,

    EVENT_INTRO,
    EVENT_OUTRO_01,
    EVENT_OUTRO_02,

    EVENT_APPLAUSE,
    EVENT_TALK,

    EVENT_SHOCKWAVE,
    EVENT_SHOCKWAVE_2,

    EVENT_TRAUMATIC_BLOW,
    EVENT_CONFLAGRATE,
    EVENT_METEOR,
    EVENT_CONFLAGRATE_2,
};

enum eTalks
{
    TALK_INTRO_01   = 0,
    TALK_INTRO_02,

    TALK_INTRO      = 0,
    TALK_AGGRO,
    TALK_DEFEATED,
    TALK_KILLING,
    TALK_OUTRO_01,
    TALK_OUTRO_02,

    TALK_00         = 0,
    TALK_01,
    TALK_02,
    TALK_03,
    TALK_04,

    TALK_AGGRO_     = 0,
    TALK_DEFEATED_,
    TALK_KILLING_,
    TALK_OUTRO_01_,
    TALK_OUTRO_02_,

    TALK_AGGRO__  = 0,
    TALK_DEFEATED__,
    TALK_KILLING__
};

enum eActions
{
    ACTION_INTRO = 1,
    ACTION_OUTRO_01,
    ACTION_OUTRO_02,

    ACTION_ENCOURAGE = 1,
    ACTION_RETIRE,
    ACTION_ATTACK,
    ACTION_ATTACK_STOP,

    ACTION_OUTRO_01_ = 1,
    ACTION_OUTRO_02_,
};

enum eStatus
{
    STATUS_ATTACK_PLAYER,
    STATUS_ATTACK_GRUNTS
};

struct mob_xian_the_weaponmaster_trigger : public ScriptedAI
{
    explicit mob_xian_the_weaponmaster_trigger(Creature* creature) : ScriptedAI(creature)
    {
        event_go = false;
        instance = creature->GetInstanceScript();
    }

    EventMap events;
    InstanceScript* instance;

    bool event_go;

    void Reset() override
    {
        event_go = false;
        me->GetMotionMaster()->MoveTargetedHome();
        SetCanSeeEvenInPassiveMode(true);
    }

    void MoveInLineOfSight(Unit* who) override
    {
        if (instance->GetBossState(DATA_TRIAL_OF_THE_KING) != DONE)
        {
            if (!event_go && who->IsPlayer() && who->GetAreaId() == 6471)
            {
                instance->SetData(TYPE_MING_INTRO, 0);
                event_go = true;
                events.RescheduleEvent(EVENT_TALK_0, 3000);
            }
        }
    }

    void UpdateAI(uint32 diff) override
    {
        events.Update(diff);

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_TALK_0:
                Talk(TALK_INTRO_01);
                me->GetMotionMaster()->MovePoint(0, -4220.277f, -2600.117f, 16.47f);
                events.RescheduleEvent(EVENT_TALK_1, 4000);
                break;
            case EVENT_TALK_1:
                me->GetMotionMaster()->MovePoint(0, -4229.333f, -2624.051f, 16.47f);
                events.RescheduleEvent(EVENT_JUMP_XIAN, 7000);
                break;
            case EVENT_JUMP_XIAN:
                Talk(TALK_INTRO_02);
                me->GetMotionMaster()->MoveJump(-4296.391f, -2613.577f, 22.325f, 30.f, 20.f);
                events.RescheduleEvent(EVENT_DISAPPEAR, 7000);
                break;
            case EVENT_DISAPPEAR:
                DoCast(SPELL_MOGU_JUMP);
                instance->SetData(TYPE_MING_ATTACK, 0);
                me->SetVisible(false);
                break;
            }
        }
    }
};

struct boss_ming_the_cunning : public BossAI
{
    explicit boss_ming_the_cunning(Creature* creature) : BossAI(creature, BOSS_MING_THE_CUNNING)
    {
        magnetic_timer = 1000;
        talk = false;
    }

    uint32 magnetic_timer;
    bool talk;

    void Reset() override
    {
        me->GetMotionMaster()->MoveTargetedHome();
    }

    void EnterEvadeMode() override
    {
        instance->SetData(TYPE_WIPE_FIRST_BOSS, 0);
    }

    void EnterCombat(Unit* /*unit*/) override
    {
        talk = false;
        Talk(TALK_AGGRO);
        DoCast(85667);
        events.RescheduleEvent(EVENT_LIGHTNING_BOLT, 3000);
        events.RescheduleEvent(EVENT_WHIRLING_DERVISH, 10000);
        events.RescheduleEvent(EVENT_MAGNETIC_FIELD, 30000);
    }

    void KilledUnit(Unit* u) override
    {
        if (!u && !u->IsPlayer())
            return;

        Talk(TALK_KILLING);
    }

    void DoAction(int32 const action) override
    {
        switch (action)
        {
        case ACTION_INTRO:
            Talk(TALK_INTRO);
            break;
        case ACTION_OUTRO_01:
            if (talk)
                return;

            talk = true;
            Talk(TALK_OUTRO_01);
            events.RescheduleEvent(EVENT_OUTRO_01, 3000);
            break;
        case ACTION_OUTRO_02:
            Talk(TALK_OUTRO_02);
            events.RescheduleEvent(EVENT_OUTRO_02, 3000);
            break;
        }
    }

    void DamageTaken(Unit* /*killer*/, uint32 &damage, DamageEffectType /*dmgType*/) override
    {
        if (int(me->GetHealth()) - int(damage) <= 0)
        {
            Talk(TALK_DEFEATED);
            damage = 0;
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
            me->GetMotionMaster()->MoveTargetedHome();
            me->AttackStop();
            events.Reset();
            instance->SetData(TYPE_MING_RETIRED, 0);
            events.RescheduleEvent(EVENT_BOSS_RETIRE, 4000);
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (me->HasAura(SPELL_MAGNETIC_FIELD))
        {
            if (magnetic_timer <= diff)
            {
                instance->instance->ApplyOnEveryPlayer([&](Player* player)
                {
                    if (player->GetDistance2d(me) <= 5.f)
                        player->GetMotionMaster()->MoveJump(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 25.0f, 10.f);
                });

                magnetic_timer = 1000;
            }
            else
                magnetic_timer -= diff;
        }

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_OUTRO_02:
                instance->SetData(TYPE_OUTRO_04, 0);
                break;
            case EVENT_OUTRO_01:
                instance->SetData(TYPE_OUTRO_02, 0);
                break;
            case EVENT_LIGHTNING_BOLT:
                if (!me->HasAura(SPELL_MAGNETIC_FIELD))
                    if (auto victim = me->getVictim())
                        DoCast(victim, SPELL_LIGHTNING_BOLT, false);

                events.RescheduleEvent(EVENT_LIGHTNING_BOLT, 6000);
                break;
            case EVENT_WHIRLING_DERVISH:
                if (!me->HasAura(SPELL_MAGNETIC_FIELD))
                    DoCast(SPELL_WHIRLING_DERVISH);

                events.RescheduleEvent(EVENT_WHIRLING_DERVISH, 10000);
                break;
            case EVENT_MAGNETIC_FIELD:
                DoCast(SPELL_MAGNETIC_FIELD);
                events.RescheduleEvent(EVENT_MAGNETIC_FIELD, 30000);
                break;
            case EVENT_BOSS_RETIRE:
                instance->SetData(TYPE_KUAI_ATTACK, 0);
                break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

struct mob_whirling_dervish : public ScriptedAI
{
    explicit mob_whirling_dervish(Creature* creature) : ScriptedAI(creature)
    {
        instance = creature->GetInstanceScript();
        DoCast(SPELL_WIRHLING_DERVISH_2);
        me->ForcedDespawn(10000);
    }

    EventMap events;
    InstanceScript* instance;

    void EnterCombat(Unit* /*unit*/) override
    {
        events.RescheduleEvent(EVENT_1, 2000);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (auto victim = me->getVictim())
            if (victim && victim->GetDistance2d(me) > 5.0f)
                return;

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_1:
                if (auto victim = me->getVictim())
                {
                    DoCast(victim, SPELL_THROW, false);
                    DoCast(victim, SPELL_THROW_2, false);
                }

                if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                    AttackStart(target);

                events.RescheduleEvent(EVENT_1, 3000);
                break;
            }
        }
    }
};

struct mob_adepts : public ScriptedAI
{
    explicit mob_adepts(Creature* creature) : ScriptedAI(creature)
    {
        instance = creature->GetInstanceScript();
        status = STATUS_ATTACK_PLAYER;
        me->SetReactState(REACT_AGGRESSIVE);
        me->setFaction(16);
    }

    EventMap events;
    InstanceScript* instance;
    uint8 status;
    float x;
    float y;

    void DoAction(int32 const action) override
    {
        switch (action)
        {
        case ACTION_ENCOURAGE:
            GetPositionWithDistInOrientation(me, 5.0f, me->GetOrientation(), x, y);
            me->GetMotionMaster()->MovePoint(0, x, y, me->GetMap()->GetHeight(x, y, me->GetPositionZ()));

            DoCast(120867);
            events.RescheduleEvent(EVENT_APPLAUSE + urand(0, 1), 500 + urand(500, 1500));
            break;
        case ACTION_RETIRE:
            GetPositionWithDistInOrientation(me, -5.0f, me->GetOrientation(), x, y);
            me->GetMotionMaster()->MovePoint(1, x, y, me->GetMap()->GetHeight(x, y, me->GetPositionZ()));

            me->RemoveAura(120867);
            DoCast(121569);
            events.Reset();
            break;
        case ACTION_ATTACK:
            status = STATUS_ATTACK_GRUNTS;

            GetPositionWithDistInOrientation(me, 30.0f, me->GetOrientation(), x, y);
            me->GetMotionMaster()->MovePoint(0, x, y, me->GetMap()->GetHeight(x, y, me->GetPositionZ()));

            me->RemoveAura(121569);
            events.Reset();
            break;
        }
    }

    void MovementInform(uint32 motionType, uint32 pointId) override
    {
        if (pointId == 1)
            me->SetFacingTo(me->GetOrientation() - M_PI);
    }

    void MoveInLineOfSight(Unit* who) override
    {
        if (who->IsPlayer() && who->GetAreaId() == 6471 && me->GetDistance2d(who) < 2.0f && who->isInFront(me) && status != STATUS_ATTACK_GRUNTS)
        {
            DoCast(who, 120035, false);
            me->AttackStop();
            Talk(TALK_00 + urand(0, 4));
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (auto victim = me->getVictim())
            if (status == STATUS_ATTACK_GRUNTS && victim && victim->ToPlayer())
                me->AttackStop();

        events.Update(diff);

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_TALK:
                events.RescheduleEvent(EVENT_APPLAUSE + urand(0, 1), 5000 + urand(500, 1500));
                break;
            }
        }
    }
};

struct boss_kuai_the_brute : public BossAI
{
    explicit boss_kuai_the_brute(Creature* creature) : BossAI(creature, BOSS_KUAI_THE_BRUTE)
    {
        talk = false;
    }

    ObjectGuid pet_guid;
    bool talk;

    void Reset() override
    {
        me->GetMotionMaster()->MoveTargetedHome();
        _Reset();

        if (auto sum = me->SummonCreature(61453, me->GetPositionX(), me->GetPositionY() + 3, me->GetPositionZ(), TEMPSUMMON_CORPSE_DESPAWN))
        {
            pet_guid = sum->GetGUID();
            sum->setFaction(me->getFaction());
        }
    }

    void KilledUnit(Unit* u) override
    {
        if (!u && !u->IsPlayer())
            return;

        Talk(TALK_KILLING_);
    }

    void EnterEvadeMode() override
    {
        instance->SetData(TYPE_WIPE_FIRST_BOSS, 1);
    }

    void DoAction(int32 const action) override
    {
        switch (action)
        {
        case ACTION_OUTRO_01:
            if (talk)
                return;

            talk = true;
            Talk(TALK_OUTRO_01_);
            events.RescheduleEvent(EVENT_OUTRO_01, 3000);
            break;
        case ACTION_OUTRO_02:
            Talk(TALK_OUTRO_02_);
            events.RescheduleEvent(EVENT_OUTRO_02, 3000);
            break;
        }
    }

    void EnterCombat(Unit* /*unit*/) override
    {
        talk = false;
        Talk(TALK_AGGRO_);
        events.RescheduleEvent(EVENT_SHOCKWAVE, 3000);

        if (auto mu_shiba = me->GetMap()->GetCreature(pet_guid))
            mu_shiba->AI()->DoAction(ACTION_ATTACK);
    }

    void DamageTaken(Unit* /*killer*/, uint32 &damage, DamageEffectType /*dmgType*/) override
    {
        if (int(me->GetHealth()) - int(damage) <= 0)
        {
            Talk(TALK_DEFEATED_);
            damage = 0;
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
            me->GetMotionMaster()->MoveTargetedHome();
            me->AttackStop();
            events.Reset();
            events.RescheduleEvent(EVENT_BOSS_RETIRE, 4000);

            instance->SetData(TYPE_KUAI_RETIRED, 0);

            if (auto mu_shiba = me->GetMap()->GetCreature(pet_guid))
            {
                if (mu_shiba && mu_shiba->isAlive())
                {
                    mu_shiba->GetMotionMaster()->MoveFollow(me, 2.0f, M_PI / 4);
                    mu_shiba->GetAI()->DoAction(ACTION_ATTACK_STOP);
                }
            }
        }
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
            case EVENT_OUTRO_02:
                instance->SetData(TYPE_OUTRO_05, 0);
                break;
            case EVENT_OUTRO_01:
                instance->SetData(TYPE_OUTRO_01, 0);
                break;
            case EVENT_SHOCKWAVE:
                if (auto victim = me->getVictim())
                    DoCast(victim, SPELL_SHOCKWAVE, false);

                ZoneTalk(5);
                me->AddUnitState(UNIT_STATE_CANNOT_TURN);
                events.RescheduleEvent(EVENT_SHOCKWAVE, 15000);
                events.RescheduleEvent(EVENT_SHOCKWAVE_2, 4000);
                break;
            case EVENT_BOSS_RETIRE:
                instance->SetData(TYPE_HAIYAN_ATTACK, 0);
                DoAction(ACTION_OUTRO_01);
                break;
            case EVENT_SHOCKWAVE_2:
                me->ClearUnitState(UNIT_STATE_CANNOT_TURN);
                break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

struct mob_mu_shiba : public ScriptedAI
{
    explicit mob_mu_shiba(Creature* creature) : ScriptedAI(creature)
    {
        me->SetReactState(REACT_PASSIVE);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
    }

    EventMap events;

    void Reset() override
    {
        DoAction(ACTION_ATTACK_STOP);
        DoCast(155085);
    }

    void EnterCombat(Unit* /*unit*/) override
    {
        events.RescheduleEvent(EVENT_1, 2000);
    }

    void DoAction(int32 const action) override
    {
        switch (action)
        {
        case ACTION_ATTACK:
            me->SetReactState(REACT_AGGRESSIVE);
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
            DoZoneInCombat();
            me->RemoveAura(155085);

            if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                AttackStart(target);
            break;
        case ACTION_ATTACK_STOP:
            events.Reset();
            me->AttackStop();
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
            break;
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
            case EVENT_1:
                if (auto victim = me->getVictim())
                    DoCast(victim, 119948, false);

                if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                    AttackStart(target);

                events.RescheduleEvent(EVENT_1, 25000);
                break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

struct boss_haiyan_the_unstoppable : public BossAI
{
    explicit boss_haiyan_the_unstoppable(Creature* creature) : BossAI(creature, BOSS_HAIYAN_THE_UNSTOPPABLE)
    {
        talk = false;
    }

    bool talk;

    void EnterCombat(Unit* /*unit*/) override
    {
        Talk(TALK_AGGRO__);
        events.RescheduleEvent(EVENT_TRAUMATIC_BLOW, 3000);
        events.RescheduleEvent(EVENT_CONFLAGRATE, 10000);
        events.RescheduleEvent(EVENT_METEOR, 30000);
    }

    void Reset() override
    {
        me->GetMotionMaster()->MoveTargetedHome();
        _Reset();
        talk = false;
    }

    void EnterEvadeMode() override
    {
        instance->SetData(TYPE_WIPE_FIRST_BOSS, 2);
    }

    void DoAction(int32 const action) override
    {
        switch (action)
        {
        case ACTION_OUTRO_01_:
            if (talk)
                return;

            talk = true;
            Talk(TALK_OUTRO_01_);
            events.RescheduleEvent(EVENT_OUTRO_01, 3000);
            break;
        case ACTION_OUTRO_02_:
            Talk(TALK_OUTRO_02_);
            break;
        }
    }

    void KilledUnit(Unit* u) override
    {
        if (!u && !u->IsPlayer())
            return;

        Talk(TALK_KILLING__);
    }

    void DamageTaken(Unit* killer, uint32 &damage, DamageEffectType /*dmgType*/) override
    {
        if (int(me->GetHealth()) - int(damage) <= 0)
        {
            Talk(TALK_DEFEATED__);
            damage = 0;
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
            me->GetMotionMaster()->MoveTargetedHome();
            me->AttackStop();
            events.Reset();
            events.RescheduleEvent(EVENT_BOSS_RETIRE, 4000);
            DoCast(SPELL_ACHIEVEMENT_CHECK);
            me->GetMap()->UpdateEncounterState(ENCOUNTER_CREDIT_CAST_SPELL, SPELL_ACHIEV_JADE_QUILEN, me, killer);

            instance->SetData(TYPE_HAIYAN_RETIRED, 0);
        }
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
            case EVENT_OUTRO_01:
                instance->SetData(TYPE_OUTRO_03, 0);
                break;
            case EVENT_TRAUMATIC_BLOW:
                if (auto victim = me->getVictim())
                    DoCast(victim, SPELL_TRAUMATIC_BLOW, false);

                events.RescheduleEvent(EVENT_TRAUMATIC_BLOW, 6000);
                break;
            case EVENT_CONFLAGRATE:
                if (auto victim = me->getVictim())
                    DoCast(victim, SPELL_CONFLAGRATE, false);

                events.RescheduleEvent(EVENT_CONFLAGRATE, 10000);
                events.RescheduleEvent(EVENT_CONFLAGRATE_2, 2000);
                break;
            case EVENT_METEOR:
                if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                    DoCast(target, SPELL_METEOR, false);

                events.RescheduleEvent(EVENT_METEOR, 30000);
                break;
            case EVENT_BOSS_RETIRE:
                instance->instance->ApplyOnEveryPlayer([](Player* player)
                {
                    player->ToPlayer()->getHostileRefManager().deleteReferences();
                    player->CombatStop();
                });
                instance->DoUpdateAchievementCriteria(CRITERIA_TYPE_SCRIPT_EVENT_2, 32859);
                instance->SetBossState(DATA_TRIAL_OF_THE_KING, DONE);
                instance->SetData(TYPE_ALL_ATTACK, 0);
                break;
            case EVENT_CONFLAGRATE_2:
                if (auto victim = me->getVictim())
                    DoCast(victim, SPELL_CONFLAGRATE_4, false);
                break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

void AddSC_boss_trial_of_the_king()
{
    RegisterCreatureAI(mob_xian_the_weaponmaster_trigger);
    RegisterCreatureAI(boss_ming_the_cunning);
    RegisterCreatureAI(mob_whirling_dervish);
    RegisterCreatureAI(mob_adepts);
    RegisterCreatureAI(boss_kuai_the_brute);
    RegisterCreatureAI(mob_mu_shiba);
    RegisterCreatureAI(boss_haiyan_the_unstoppable);
}