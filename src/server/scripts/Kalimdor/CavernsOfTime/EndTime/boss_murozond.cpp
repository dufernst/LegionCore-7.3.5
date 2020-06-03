#include "end_time.h"
#include "Group.h"

enum MurozondScriptTexts
{
    SAY_AGGRO   = 0,
    SAY_DEATH   = 1,
    SAY_GLASS_1 = 2,
    SAY_GLASS_2 = 3,
    SAY_GLASS_3 = 4,
    SAY_GLASS_4 = 5,
    SAY_GLASS_5 = 6,
    SAY_INTRO_1 = 7,
    SAY_INTRO_2 = 8,
    SAY_KILL    = 9
};

enum NozdormuScriptTexts
{
    SAY_NOZDORMU_INTRO      = 0,
    SAY_NOZDORMU_OUTRO_1    = 1,
    SAY_NOZDORMU_OUTRO_2    = 2,
    SAY_NOZDORMU_OUTRO_3    = 3,
    SAY_NOZDORMU_OUTRO_4    = 4,
    SAY_NOZDORMU_OUTRO_5    = 5
};

enum Spells
{
    SPELL_INFINITE_BREATH               = 102569,
    SPELL_TEMPORAL_BLAST                = 102381,
    SPELL_TAIL_SWEEP                    = 108589,
    SPELL_FADING                        = 107550,
    SPELL_DISTORTION_BOMB               = 102516,
    SPELL_DISTORTION_BOMB_DMG           = 101984,
    SPELL_SANDS_OF_THE_HOURGLASS        = 102668,
    SPELL_TEMPORAL_SNAPSHOT             = 101592,
    SPELL_REWIND_TIME                   = 101590,
    SPELL_BLESSING_OF_BRONZE_DRAGONS    = 102364,
    SPELL_KILL_MUROZOND                 = 110158
};

enum Events
{
    EVENT_INFINITE_BREATH   = 1,
    EVENT_TEMPORAL_BLAST    = 2,
    EVENT_TAIL_SWEEP        = 3,
    EVENT_DESPAWN           = 4,
    EVENT_DISTORTION_BOMB   = 5,
    EVENT_CHECK_ADDS        = 6,
    EVENT_INTRO_1           = 7,
    EVENT_INTRO_2           = 8,
    EVENT_INTRO_3           = 9,
    EVENT_NOZDORMU_INTRO    = 10,
    EVENT_CONTINUE          = 11,
    EVENT_NOZDORMU_OUTRO_1  = 12,
    EVENT_NOZDORMU_OUTRO_2  = 13,
    EVENT_NOZDORMU_OUTRO_3  = 14,
    EVENT_NOZDORMU_OUTRO_4  = 15,
    EVENT_NOZDORMU_OUTRO_5  = 16
};

enum Adds
{
    NPC_MIRROR  = 54435
};

enum Other
{
    POINT_LAND          = 1,
    ACTION_HOURGLASS    = 2,
    TYPE_HOURGLASS      = 3,
    ACTION_NOZDORMU     = 4
};

const Position landPos = { 4169.71f, -433.40f, 120.0f, 2.59f };

struct boss_murozond : public BossAI
{
    explicit boss_murozond(Creature* creature) : BossAI(creature, DATA_MUROZOND)
    {
        me->setActive(true);
        bDead = false;
        phase = 0;
        hourglass = 5;
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
    }

    uint8 phase;
    bool bDead;
    uint32 hourglass;

    void ActivateMirrors()
    {
        std::list<Creature*> mirrorList;
        me->GetCreatureListWithEntryInGrid(mirrorList, NPC_MIRROR, 500.0f);

        if (mirrorList.empty())
            return;

        for (auto& mirrors : mirrorList)
        {
            if (auto mirror = mirrors->ToCreature())
                if (mirror->isAlive() && mirror->IsInWorld())
                    mirror->AI()->DoAction(ACTION_HOURGLASS);
        }
    }

    void Reset() override
    {
        _Reset();
        me->SetReactState(REACT_AGGRESSIVE);
        bDead = false;
        hourglass = 5;
        me->RemoveAllDynObjects();
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_SANDS_OF_THE_HOURGLASS);
    }

    void JustDied(Unit* /*killer*/) override
    {
        _JustDied();
    }

    uint32 GetData(uint32 type) const override
    {
        if (type == TYPE_HOURGLASS)
            return hourglass;
        return 0;
    }

    void MoveInLineOfSight(Unit* who) override
    {
        if (phase)
            return;

        if (me->GetDistance2d(who) >= 60.0f)
            return;

        phase = 1;
        events.ScheduleEvent(EVENT_INTRO_1, 2000);
        events.ScheduleEvent(EVENT_INTRO_2, 28000);
        events.ScheduleEvent(EVENT_INTRO_3, 48000);
    }

    void DoAction(int32 const action) override
    {
        if (action == ACTION_HOURGLASS)
        {
            if (hourglass == 0)
                return;

            ActivateMirrors();
            me->RemoveAllDynObjects();
            me->AttackStop();
            me->SetReactState(REACT_PASSIVE);
            hourglass--;
            instance->DoSetAlternatePowerOnPlayers(hourglass);
            events.ScheduleEvent(EVENT_CONTINUE, 5000);
            events.RescheduleEvent(EVENT_INFINITE_BREATH, urand(14000, 24000));
            events.RescheduleEvent(EVENT_TAIL_SWEEP, 14000);
            events.RescheduleEvent(EVENT_TEMPORAL_BLAST, 9000);
            events.RescheduleEvent(EVENT_DISTORTION_BOMB, urand(9000, 12000));
        }
    }

    void EnterCombat(Unit* /*who*/) override
    {
        _EnterCombat();
        Talk(SAY_AGGRO);
        events.ScheduleEvent(EVENT_INFINITE_BREATH, urand(10000, 20000));
        events.ScheduleEvent(EVENT_TAIL_SWEEP, 10000);
        events.ScheduleEvent(EVENT_TEMPORAL_BLAST, 5000);
        events.ScheduleEvent(EVENT_NOZDORMU_INTRO, 5000);
        events.ScheduleEvent(EVENT_DISTORTION_BOMB, urand(5000, 8000));

        DoCast(SPELL_TEMPORAL_SNAPSHOT);
        instance->DoCastSpellOnPlayers(SPELL_BLESSING_OF_BRONZE_DRAGONS);
        instance->DoCastSpellOnPlayers(SPELL_SANDS_OF_THE_HOURGLASS);
        instance->SetBossState(DATA_MUROZOND, IN_PROGRESS);
        me->RemoveAllDynObjects();
        DoZoneInCombat();
    }

    void KilledUnit(Unit* /*who*/) override
    {
        Talk(SAY_KILL);
    }

    void CompleteEncounter()
    {
        if (instance)
        {
            // Achievement
            instance->DoUpdateAchievementCriteria(CRITERIA_TYPE_BE_SPELL_TARGET, SPELL_KILL_MUROZOND, 0, 0, me);

            // Guild Achievement
            instance->instance->ApplyOnEveryPlayer([&](Player* player)
            {
                if (Group* group = player->GetGroup())
                    if (player->GetGuildId() && group->IsGuildGroup(player->GetGuildGUID(), true, true))
                        group->UpdateGuildAchievementCriteria(CRITERIA_TYPE_BE_SPELL_TARGET, SPELL_KILL_MUROZOND, 0, 0, NULL, me);
            });

            me->GetMap()->UpdateEncounterState(ENCOUNTER_CREDIT_CAST_SPELL, SPELL_KILL_MUROZOND, me, me);

            if (auto go = ObjectAccessor::GetGameObject(*me, instance->GetGuidData(DATA_HOURGLASS)))
                go->SetFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_INTERACT_COND);
        }
    }

    void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType /*dmgType*/) override
    {
        if (!bDead && me->HealthBelowPct(8))
        {
            bDead = true;
            me->InterruptNonMeleeSpells(false);
            me->RemoveAllAuras();
            DoCast(me, SPELL_FADING, true);
            CompleteEncounter();
            events.ScheduleEvent(EVENT_DESPAWN, 3000);
            Talk(SAY_DEATH);
        }
    }

    void JustReachedHome() override
    {
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_SANDS_OF_THE_HOURGLASS);
        BossAI::JustReachedHome();
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim() && phase != 1)
            return;

        events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_INTRO_1:
                Talk(SAY_INTRO_1);
                break;
            case EVENT_INTRO_2:
                Talk(SAY_INTRO_2);
                break;
            case EVENT_INTRO_3:
                phase = 2;
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                break;
            case EVENT_NOZDORMU_INTRO:
                if (auto nozdormu = ObjectAccessor::GetCreature(*me, instance->GetGuidData(DATA_NOZDORMU)))
                    nozdormu->AI()->Talk(SAY_NOZDORMU_INTRO);
                break;
            case EVENT_DISTORTION_BOMB:
                if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 1, 100.0f, true))
                    DoCast(target, SPELL_DISTORTION_BOMB, false);

                events.ScheduleEvent(EVENT_DISTORTION_BOMB, urand(5000, 8000));
                break;
            case EVENT_INFINITE_BREATH:
                if (auto victim = me->getVictim())
                    DoCast(victim, SPELL_INFINITE_BREATH, false);

                events.ScheduleEvent(EVENT_INFINITE_BREATH, 15000);
                break;
            case EVENT_TAIL_SWEEP:
                DoCast(SPELL_TAIL_SWEEP);
                events.ScheduleEvent(EVENT_TAIL_SWEEP, 10000);
                break;
            case EVENT_TEMPORAL_BLAST:
                DoCast(SPELL_TEMPORAL_BLAST);
                events.ScheduleEvent(EVENT_TEMPORAL_BLAST, urand(14000, 18000));
                break;
            case EVENT_CONTINUE:
                Talk(SAY_GLASS_5 - hourglass);
                me->SetReactState(REACT_AGGRESSIVE);

                if (auto victim = me->getVictim())
                    me->GetMotionMaster()->MoveChase(victim);
                break;
            case EVENT_DESPAWN:
                if (auto nozdormu = ObjectAccessor::GetCreature(*me, instance->GetGuidData(DATA_NOZDORMU)))
                    nozdormu->AI()->DoAction(ACTION_NOZDORMU);

                me->RemoveAllDynObjects();
                instance->SetBossState(DATA_MUROZOND, DONE);
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_SANDS_OF_THE_HOURGLASS);

                if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                    target->Kill(me);
                break;
            default:
                break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

struct npc_murozond_mirror_image : public ScriptedAI
{
    explicit npc_murozond_mirror_image(Creature* creature) : ScriptedAI(creature)
    {
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
        me->SetReactState(REACT_PASSIVE);
        instance = me->GetInstanceScript();
    }

    InstanceScript* instance;

    void Reset() override
    {
        if (auto owner = me->GetAnyOwner())
        {
            owner->CastSpell(me, 102284, true);
            owner->CastSpell(me, 102288, true);
        }
    }

    void DoAction(int32 const action) override
    {
        if (action == ACTION_HOURGLASS)
        {
            if (auto owner = me->GetAnyOwner())
            {
                Player* m_owner = owner->ToPlayer();
                if (!m_owner)
                    return;

                if (!m_owner->isAlive())
                    m_owner->ResurrectPlayer(1.0f, false);

                m_owner->RemoveAura(SPELL_TEMPORAL_BLAST);

                m_owner->SetHealth(m_owner->GetMaxHealth());
                m_owner->SetPower(m_owner->getPowerType(), m_owner->GetMaxPower(m_owner->getPowerType()));

                m_owner->CastSpell(m_owner, SPELL_BLESSING_OF_BRONZE_DRAGONS, true);
                m_owner->NearTeleportTo(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation(), true);
            }
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (!instance || instance->GetBossState(DATA_MUROZOND) != IN_PROGRESS)
        {
            me->DespawnOrUnsummon();
            return;
        }
    }
};

struct npc_end_time_nozdormu : public ScriptedAI
{
    explicit npc_end_time_nozdormu(Creature* creature) : ScriptedAI(creature)
    {
        me->SetVisible(false);
        me->setActive(true);
        bTalk = false;
    }

    bool bTalk;
    EventMap events;

    void Reset() override
    {
        events.Reset();
    }

    void DoAction(int32 const action) override
    {
        if (!bTalk && action == ACTION_NOZDORMU)
        {
            bTalk = true;
            me->SetVisible(true);
            events.ScheduleEvent(EVENT_NOZDORMU_OUTRO_1, 5000);
            events.ScheduleEvent(EVENT_NOZDORMU_OUTRO_2, 20000);
            events.ScheduleEvent(EVENT_NOZDORMU_OUTRO_3, 33000);
            events.ScheduleEvent(EVENT_NOZDORMU_OUTRO_4, 41000);
            events.ScheduleEvent(EVENT_NOZDORMU_OUTRO_5, 44000);
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (!bTalk)
            return;

        events.Update(diff);

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_NOZDORMU_OUTRO_1:
                Talk(SAY_NOZDORMU_OUTRO_1);
                break;
            case EVENT_NOZDORMU_OUTRO_2:
                Talk(SAY_NOZDORMU_OUTRO_2);
                break;
            case EVENT_NOZDORMU_OUTRO_3:
                Talk(SAY_NOZDORMU_OUTRO_3);
                break;
            case EVENT_NOZDORMU_OUTRO_4:
                if (auto go = me->FindNearestGameObject(HOURGLASS_OF_TIME, 300.0f))
                    me->SetFacingToObject(go);
                break;
            case EVENT_NOZDORMU_OUTRO_5:
                Talk(SAY_NOZDORMU_OUTRO_5);
                bTalk = false;
                break;
            }
        }
    }
};

class go_murozond_hourglass_of_time : public GameObjectScript
{
    public:
        go_murozond_hourglass_of_time() : GameObjectScript("go_murozond_hourglass_of_time") {}

        bool OnGossipHello(Player* player, GameObject* go)
        {
            InstanceScript* instance = go->GetInstanceScript();
            if (!instance || instance->GetBossState(DATA_MUROZOND) != IN_PROGRESS)
                return true;

            if (auto murozond = ObjectAccessor::GetCreature(*go, instance->GetGuidData(DATA_MUROZOND)))
            {
                if (murozond->AI()->GetData(TYPE_HOURGLASS) == 0)
                    return true;

                murozond->AI()->DoAction(ACTION_HOURGLASS);
            }
            return false;
        }
};

void AddSC_boss_murozond()
{
    RegisterCreatureAI(boss_murozond);
    RegisterCreatureAI(npc_end_time_nozdormu);
    RegisterCreatureAI(npc_murozond_mirror_image);
    new go_murozond_hourglass_of_time();
}
