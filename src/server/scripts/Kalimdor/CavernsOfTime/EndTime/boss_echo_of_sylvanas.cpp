#include "end_time.h"

enum ScriptTexts
{
    SAY_AGGRO   = 0,
    SAY_DEATH   = 1,
    SAY_KILL    = 3,
    SAY_WIPE    = 2,
    SAY_SPELL   = 4
};

enum Spells
{
    SPELL_SHRIEK_OF_THE_HIGHBORNE       = 101412,
    SPELL_UNHOLY_SHOT                   = 101411,
    SPELL_BLACK_ARROW                   = 101404,
    SPELL_WRACKING_PAIN_ANY             = 100865,
    SPELL_WRACKING_PAIN_AURA            = 101258,
    SPELL_WRACKING_PAIN_DMG             = 101257,
    SPELL_DEATH_GRIP_AOE                = 101397,
    SPELL_DEATH_GRIP                    = 101987,
    SPELL_SUMMON_GHOULS                 = 102603, // before combat
    SPELL_TELEPORT                      = 101398, 
    SPELL_CALL_OF_THE_HIGHBORNE         = 100686, // immune
    SPELL_CALL_OF_THE_HIGHBORNE_1       = 100867, // visual spawn ghouls
    SPELL_CALL_OF_THE_HIGHBORNE_2       = 105766, // visual back ghouls
    SPELL_CALL_OF_THE_HIGHBORNE_3       = 102581,
    SPELL_SPAWN_GHOUL                   = 101200,
    SPELL_SEEPING_SHADOWS_DUMMY         = 103175,
    SPELL_SEEPING_SHADOWS_AURA          = 103182,
    SPELL_SACRIFICE                     = 101348,
    SPELL_DWINDLE                       = 101259,
    SPELL_JUMP                          = 101517,
    SPELL_JUMP_SCRIPT                   = 101527,
    SPELL_JUMP_VEHICLE                  = 101528,
    SPELL_PERMANENT_FEIGH_DEATH         = 96733,
    SPELL_SHRINK                        = 101318,
    SPELL_BLIGHTED_ARROWS               = 101401
};

enum Events
{
    EVENT_SHRIEK_OF_THE_HIGHBORNE   = 1,
    EVENT_UNHOLY_SHOT               = 2,
    EVENT_BLACK_ARROW               = 3,
    EVENT_DEATH_GRIP                = 4,
    EVENT_TELEPORT                  = 5,
    EVENT_TELEPORT_1                = 6,
    EVENT_SPAWN_GHOUL               = 7,
    EVENT_MOVE_GHOUL                = 8,
    EVENT_FALL                      = 0,
    EVENT_START                     = 10,
    EVENT_BLIGHTED_ARROWS           = 11
};

enum Adds
{
    NPC_GHOUL_1         = 54197,
    NPC_BRITTLE_GHOUL   = 54952,
    NPC_RISEN_GHOUL     = 54191,
    NPC_JUMP            = 54385
};

enum Others
{
    DATA_GUID           = 1,
    POINT_GHOUL         = 2,
    ACTION_GHOUL        = 3,
    ACTION_KILL_GHOUL   = 4
};

const Position centerPos = {3845.51f, 909.318f, 56.1463f, 1.309f};

Position const ghoulPos[8] =
{
    { 3810.03f, 914.043f, 55.3974f, 0.0f },
    { 3818.82f, 892.83f, 56.0076f, 0.7854f },
    { 3840.03f, 884.043f, 56.0712f, 1.5708f },
    { 3861.24f, 892.83f, 56.0712f, 2.35619f },
    { 3870.03f, 914.043f, 56.0277f, 3.14159f },
    { 3861.24f, 935.256f, 55.9664f, 3.92699f },
    { 3840.03f, 944.043f, 55.9664f, 4.71239f },
    { 3818.82f, 935.256f, 56.0161f, 5.49779f }
};

struct boss_echo_of_sylvanas : public BossAI
{
    explicit boss_echo_of_sylvanas(Creature* creature) : BossAI(creature, DATA_ECHO_OF_SYLVANAS)
    {
        me->setActive(true);
        me->CastSpell(me, SPELL_SUMMON_GHOULS, true);
        DoCast(SPELL_CALL_OF_THE_HIGHBORNE_3);
    }

    uint8 ghouls;
    uint8 deadghouls;
    ObjectGuid _first;
    ObjectGuid _prev;

    void Reset() override
    {
        _Reset();
        me->SetReactState(REACT_AGGRESSIVE);
        me->SetDisableGravity(false);
        me->SetCanFly(false);
        ghouls = 0;
        deadghouls = 0;
    }

    void JustDied(Unit* /*killer*/) override
    {
        _JustDied();
        Talk(SAY_DEATH);

        // Quest
        instance->instance->ApplyOnEveryPlayer([&](Player* player)
        {
            if (me->GetDistance2d(player) <= 50.0f && player->GetQuestStatus(30097) == QUEST_STATUS_INCOMPLETE)
                DoCast(player, SPELL_ARCHIVED_SYLVANAS, true);
        });
    }

    void EnterCombat(Unit* /*who*/) override
    {
        _EnterCombat();
        Talk(SAY_AGGRO);
        events.ScheduleEvent(EVENT_UNHOLY_SHOT, urand(5000, 20000));
        events.ScheduleEvent(EVENT_SHRIEK_OF_THE_HIGHBORNE, urand(5000, 20000));
        events.ScheduleEvent(EVENT_TELEPORT, 5000);
        events.ScheduleEvent(EVENT_BLACK_ARROW, 3000);
        me->RemoveAura(SPELL_SUMMON_GHOULS);
        me->RemoveAura(SPELL_CALL_OF_THE_HIGHBORNE_3);
        summons.DespawnEntry(NPC_BRITTLE_GHOUL);
        deadghouls = 0;
        DoZoneInCombat();
        instance->SetBossState(DATA_ECHO_OF_SYLVANAS, IN_PROGRESS);
    }

    void KilledUnit(Unit* who) override
    {
        if (!who)
            return;

        if (who->IsPlayer())
            Talk(SAY_KILL);
    }

    void DoAction(int32 const action) override
    {
        if (action == ACTION_GHOUL)
        {
            ghouls++;

            if (ghouls >= 1)
            {
                DoCast(SPELL_SACRIFICE);
                summons.DespawnEntry(NPC_RISEN_GHOUL);
                events.ScheduleEvent(EVENT_START, 2000);
                ghouls = 0;
            }
        }
        else if (action == ACTION_KILL_GHOUL)
            deadghouls++;
    }

    void JustReachedHome() override
    {
        Talk(SAY_WIPE);
        DoCast(me, SPELL_SUMMON_GHOULS, true);
        DoCast(SPELL_CALL_OF_THE_HIGHBORNE_3);
    }

    bool AllowAchieve()
    {
        return deadghouls >= 2;
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_UNHOLY_SHOT:
                if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                    DoCast(target, SPELL_UNHOLY_SHOT, false);

                events.ScheduleEvent(EVENT_UNHOLY_SHOT, urand(10000, 20000));
                break;
            case EVENT_BLACK_ARROW:
                if (auto* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                    DoCast(target, SPELL_BLACK_ARROW, false);

                events.ScheduleEvent(EVENT_BLACK_ARROW, 25000);
                break;
            case EVENT_BLIGHTED_ARROWS:
                break;
            case EVENT_SHRIEK_OF_THE_HIGHBORNE:
                if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                    DoCast(target, SPELL_SHRIEK_OF_THE_HIGHBORNE, false);

                events.ScheduleEvent(EVENT_SHRIEK_OF_THE_HIGHBORNE, urand(15000, 21000));
                break;
            case EVENT_TELEPORT:
                events.CancelEvent(EVENT_UNHOLY_SHOT);
                events.CancelEvent(EVENT_SHRIEK_OF_THE_HIGHBORNE);
                me->SetReactState(REACT_PASSIVE);
                me->AttackStop();
                me->InterruptNonMeleeSpells(false);
                DoCast(me, SPELL_TELEPORT, true);
                events.ScheduleEvent(EVENT_TELEPORT_1, 2000);
                break;
            case EVENT_TELEPORT_1:
                Talk(SAY_SPELL);
                DoCast(me, SPELL_CALL_OF_THE_HIGHBORNE, true);

                for (uint8 i = 0; i < 8; ++i)
                    me->SummonCreature(NPC_GHOUL_1, ghoulPos[i]);

                events.ScheduleEvent(EVENT_DEATH_GRIP, 3000);
                events.ScheduleEvent(EVENT_SPAWN_GHOUL, 3000);
                break;
            case EVENT_DEATH_GRIP:
                DoCast(SPELL_DEATH_GRIP_AOE);
                break;
            case EVENT_SPAWN_GHOUL:
            {
                deadghouls = 0;

                for (uint8 i = 0; i < 8; ++i)
                {
                    if (auto ghoul = me->SummonCreature(NPC_RISEN_GHOUL, ghoulPos[i]))
                    {
                        if (auto prev = ObjectAccessor::GetUnit(*me, _prev))
                        {
                            prev->CastSpell(ghoul, SPELL_WRACKING_PAIN_ANY, true);
                            prev->GetAI()->SetGUID(prev->GetGUID(), DATA_GUID);

                        }

                        _prev = ghoul->GetGUID();

                        if (i == 0)
                            _first = ghoul->GetGUID();
                    }
                }

                if (auto first = ObjectAccessor::GetUnit(*me, _first))
                {
                    if (auto prev = ObjectAccessor::GetUnit(*me, _prev))
                    {
                        prev->CastSpell(first, SPELL_WRACKING_PAIN_ANY, true);
                        prev->GetAI()->SetGUID(first->GetGUID(), DATA_GUID);
                    }
                }

                summons.DespawnEntry(NPC_GHOUL_1);
                break;
            }
            case EVENT_START:
                me->RemoveAura(SPELL_CALL_OF_THE_HIGHBORNE);
                me->SetReactState(REACT_AGGRESSIVE);

                if (auto victim = me->getVictim())
                    me->GetMotionMaster()->MoveChase(victim);

                events.ScheduleEvent(EVENT_UNHOLY_SHOT, urand(5000, 20000));
                events.ScheduleEvent(EVENT_SHRIEK_OF_THE_HIGHBORNE, urand(5000, 20000));
                events.ScheduleEvent(EVENT_TELEPORT, 40000);
                break;
            default:
                break;
            }
        }

        DoMeleeAttackIfReady();
    }
};

struct npc_echo_of_sylvanas_ghoul : public ScriptedAI
{
    explicit npc_echo_of_sylvanas_ghoul(Creature* creature) : ScriptedAI(creature)
    {
        me->SetReactState(REACT_PASSIVE);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
    }

    EventMap events;

    void Reset() override
    {
        events.Reset();
    }

    void IsSummonedBy(Unit* /*owner*/) override
    {
        DoCast(me, SPELL_CALL_OF_THE_HIGHBORNE_1, true);
    }
};

struct npc_echo_of_sylvanas_risen_ghoul : public ScriptedAI
{
    explicit npc_echo_of_sylvanas_risen_ghoul(Creature* pCreature) : ScriptedAI(pCreature)
    {
        me->SetReactState(REACT_PASSIVE);
    }

    EventMap events;
    ObjectGuid _guid;

    void Reset() override
    {
        events.Reset();
        _guid.Clear();
        me->SetFloatValue(UNIT_FIELD_BOUNDING_RADIUS, 5.0f);
        me->SetFloatValue(UNIT_FIELD_COMBAT_REACH, 5.0f);
    }

    void JustDied(Unit* /*killer*/) override
    {
        me->RemoveAura(SPELL_WRACKING_PAIN_ANY);
        if (auto target = ObjectAccessor::GetCreature(*me, _guid))
            target->RemoveAura(SPELL_WRACKING_PAIN_ANY);

        _guid.Clear();

        if (auto sylvanas = me->FindNearestCreature(NPC_ECHO_OF_SYLVANAS, 300.0f))
            sylvanas->GetAI()->DoAction(ACTION_KILL_GHOUL);

        me->DespawnOrUnsummon(500);
    }

    void EnterCombat(Unit* /*who*/) override
    {
        DoCast(me, SPELL_SEEPING_SHADOWS_DUMMY, true);
        events.ScheduleEvent(EVENT_MOVE_GHOUL, 2000);
    }

    void SetGUID(ObjectGuid const& guid, int32 type)
    {
        if (type == DATA_GUID)
            _guid = guid;
    }

    ObjectGuid GetGUID(int32 type)
    {
        if (type == DATA_GUID)
            return _guid;
        return
            ObjectGuid::Empty;
    }

    void MovementInform(uint32 type, uint32 data) override
    {
        if (type == POINT_MOTION_TYPE)
            if (data == POINT_GHOUL)
                if (auto sylvanas = me->FindNearestCreature(NPC_ECHO_OF_SYLVANAS, 300.0f))
                    sylvanas->GetAI()->DoAction(ACTION_GHOUL);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_MOVE_GHOUL:
                me->SetWalk(true);
                me->SetSpeed(MOVE_RUN, 0.5f, true);
                me->SetSpeed(MOVE_WALK, 0.5f, true);
                me->GetMotionMaster()->MovePoint(POINT_GHOUL, centerPos);
                DoCast(me, SPELL_WRACKING_PAIN_AURA, true);
                break;
            }
        }
    }
};

class spell_echo_of_sylvanas_wracking_pain_dmg : public SpellScriptLoader
{
    public:
        spell_echo_of_sylvanas_wracking_pain_dmg() : SpellScriptLoader("spell_echo_of_sylvanas_wracking_pain_dmg") {}

        class spell_echo_of_sylvanas_wracking_pain_dmg_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_echo_of_sylvanas_wracking_pain_dmg_SpellScript);

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                if (!GetCaster() || !GetCaster()->isAlive())
                {
                    targets.clear();
                    return;
                }

                if (!GetCaster()->GetAI())
                {
                    targets.clear();
                    return;
                }

                ObjectGuid _guid = GetCaster()->GetAI()->GetGUID(DATA_GUID);
                if (auto pTarget = ObjectAccessor::GetCreature(*GetCaster(), _guid))
                {
                    if (pTarget->isAlive())
                        targets.remove_if(WrackingPainTargetSelector(GetCaster(), pTarget));
                    else
                        targets.clear();
                }
                else
                    targets.clear();
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_echo_of_sylvanas_wracking_pain_dmg_SpellScript::FilterTargets, EFFECT_0,TARGET_UNIT_SRC_AREA_ENEMY);
            }

        private:

            class WrackingPainTargetSelector
            {
                public:
                    WrackingPainTargetSelector(Unit* caster, Unit* target) : i_caster(caster), i_target(target) {}

                    bool operator()(WorldObject* unit)
                    {
                        if (unit->IsInBetween(i_caster, i_target))
                            return false;
                        return true;
                    }

                    Unit* i_caster;
                    Unit* i_target;
            };
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_echo_of_sylvanas_wracking_pain_dmg_SpellScript();
        }
};

class spell_echo_of_sylvanas_death_grip_aoe : public SpellScript
{
    PrepareSpellScript(spell_echo_of_sylvanas_death_grip_aoe);

    void HandleScript(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster)
            return;
        if (!target)
            return;

        target->CastSpell(caster, SPELL_DEATH_GRIP, true);
    }

    void Register()
    {
        OnEffectHitTarget += SpellEffectFn(spell_echo_of_sylvanas_death_grip_aoe::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

class spell_echo_of_sylvanas_seeping_shadows : public AuraScript
{
    PrepareAuraScript(spell_echo_of_sylvanas_seeping_shadows);

    void HandlePeriodicTick(AuraEffect const* /*aurEff*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        int32 amount = int32(0.2f * (100.0f - caster->GetHealthPct()));

        if (Aura* aur = caster->GetAura(103182))
            aur->ModStackAmount(amount - aur->GetStackAmount());
        else
            caster->CastCustomSpell(103182, SPELLVALUE_AURA_STACK, amount, caster, true);
    }

    void Register()
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_echo_of_sylvanas_seeping_shadows::HandlePeriodicTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }
};

/*class achievement_several_ties : public AchievementCriteriaScript
{
    public:
        achievement_several_ties() : AchievementCriteriaScript("achievement_several_ties") {}

        bool OnCheck(Player* source, Unit* target)
        {
            if (!target)
                return false;

            if (boss_echo_of_sylvanas::boss_echo_of_sylvanasAI* echo_of_sylvanasAI = CAST_AI(boss_echo_of_sylvanas::boss_echo_of_sylvanasAI, target->GetAI()))
                return echo_of_sylvanasAI->AllowAchieve();

            return false;
        }
};*/

void AddSC_boss_echo_of_sylvanas()
{
    RegisterCreatureAI(boss_echo_of_sylvanas);
    RegisterCreatureAI(npc_echo_of_sylvanas_ghoul);
    RegisterCreatureAI(npc_echo_of_sylvanas_risen_ghoul);
    RegisterSpellScript(spell_echo_of_sylvanas_death_grip_aoe);
    RegisterAuraScript(spell_echo_of_sylvanas_seeping_shadows);
    new spell_echo_of_sylvanas_wracking_pain_dmg();
    //new achievement_several_ties();
}
