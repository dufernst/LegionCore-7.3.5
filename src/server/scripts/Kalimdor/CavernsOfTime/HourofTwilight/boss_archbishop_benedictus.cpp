#include "Vehicle.h"
#include "hour_of_twilight.h"

enum ScriptTexts
{
    SAY_DEATH       = 0,
    SAY_EVENT_1     = 1,
    SAY_EVENT_2     = 2,
    SAY_LIGHT       = 6,
    SAY_TWILIGHT    = 4
};

enum Spells
{
    SPELL_HOLY_WALL                     = 102629,

    SPELL_SMITE                         = 104503,
    SPELL_RIGHTEOUS_SNEAR_AOE           = 103149,
    SPELL_RIGHTEOUS_SNEAR_AURA          = 103151,
    SPELL_RIGHTEOUS_SNEAR_DMG           = 103161,

    SPELL_PURIFYING_LIGHT               = 103565,
    SPELL_PURIFYING_LIGHT_TARGETING     = 103600,
    SPELL_PURIFYING_LIGHT_GROW          = 103579,
    SPELL_PURIFYING_LIGHT_SUMMON_1      = 103584,
    SPELL_PURIFYING_LIGHT_SUMMON_2      = 103585,
    SPELL_PURIFYING_LIGHT_SUMMON_3      = 103586,
    SPELL_PURIFYING_LIGHT_DUMMY         = 103578,
    SPELL_PURIFYING_BLAST               = 103648,
    SPELL_PURIFYING_BLAST_DMG           = 103651,
    SPELL_PURIFIED                      = 103654,
    SPELL_PURIFIED_DMG                  = 103653,

    SPELL_UNSTABLE_TWILIGHT_DUMMY       = 103766,

    SPELL_TWILIGHT_EPIPHANY             = 103754,
    SPELL_TWILIGHT_EPIPHANY_DMG         = 103755,
    SPELL_ENGULFING_TWILIGHT            = 103762,
    SPELL_TRANSFORM                     = 103765,

    SPELL_TWILIGHT_BLAST                = 104504,

    SPELL_TWILIGHT_SNEAR_AOE            = 103362,
    SPELL_TWILIGHT_SNEAR_AURA           = 103363,
    SPELL_TWILIGHT_SNEAR_DMG            = 103526,

    SPELL_CORRUPTING_TWILIGHT           = 103767,
    SPELL_CORRUPTING_TWILIGHT_TARGETING = 103768,
    SPELL_CORRUPTING_TWILIGHT_GROW      = 103773,
    SPELL_CORRUPTING_TWILIGHT_SUMMON_1  = 103770,
    SPELL_CORRUPTING_TWILIGHT_SUMMON_2  = 103771,
    SPELL_CORRUPTING_TWILIGHT_SUMMON_3  = 103772,
    SPELL_CORRUPTING_TWILIGHT_DUMMY     = 103769,
    SPELL_TWILIGHT_BOLT                 = 103776,
    SPELL_TWILIGHT_BOLT_DMG             = 103777,
    SPELL_TWILIGHT                      = 103774,
    SPELL_TWILIGHT_DMG                  = 103775,

    SPELL_WAVE_OF_TWILIGHT              = 103780,
    SPELL_WAVE_OF_TWILIGHT_DMG          = 103781
};

enum Events
{
    EVENT_WAVE_OF_VIRTUE        = 1,
    EVENT_PURIFYING_LIGHT       = 2,
    EVENT_SMITE                 = 3,
    EVENT_RIGHTEOUS_SNEAR       = 4,
    EVENT_TWILIGHT_BLAST        = 5,
    EVENT_WAVE_OF_TWILIGHT      = 6,
    EVENT_CORRUPTING_TWILIGHT   = 7,
    EVENT_TWILIGHT_SNEAR        = 8,
    EVENT_CONTINUE              = 9,
    EVENT_JUMP_2                = 10
};

enum Adds
{
    NPC_TWILIGHT_SPARK      = 55466,
    NPC_PURIFYING_LIGHT     = 55377,
    NPC_PURIFYING_BLAST     = 55427,
    NPC_WAVE_OF_VIRTUE      = 55551,
    NPC_CORRUPTING_TWILIGHT = 55467,
    NPC_TWILIGHT_BLAST      = 55468,
    NPC_WAVE_OF_TWILIGHT    = 55469
};

enum Actions
{
    ACTION_LIGHT    = 1,
    ACTION_TWILIGHT = 2
};

class boss_archbishop_benedictus : public CreatureScript
{
    public:
        boss_archbishop_benedictus() : CreatureScript("boss_archbishop_benedictus") {}

        CreatureAI* GetAI(Creature* creature) const
        {
            return GetInstanceAI<boss_archbishop_benedictusAI>(creature);
        }

        struct boss_archbishop_benedictusAI : public BossAI
        {
            boss_archbishop_benedictusAI(Creature* creature) : BossAI(creature, DATA_BENEDICTUS)
            {
                me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_STUN, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FEAR, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_ROOT, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FREEZE, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_POLYMORPH, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_HORROR, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_SAPPED, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_CHARM, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_DISORIENTED, true);
                me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_CONFUSE, true);
                me->setActive(true);
                me->setFaction(14);
                bPhase = false;
            }

            void Reset() override
            {
                _Reset();

                bPhase = false;

                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_TWILIGHT_EPIPHANY_DMG);
                DespawnCreatures(NPC_PURIFYING_BLAST);
                DespawnCreatures(NPC_TWILIGHT_BLAST);

            }

            void EnterCombat(Unit* /*who*/) override
            {
                bPhase = false;

                events.ScheduleEvent(EVENT_PURIFYING_LIGHT, 10000);
                events.ScheduleEvent(EVENT_SMITE, urand(1000, 2000));
                events.ScheduleEvent(EVENT_RIGHTEOUS_SNEAR, urand(5000, 10000));

                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_TWILIGHT_EPIPHANY_DMG);
                instance->SetBossState(DATA_BENEDICTUS, IN_PROGRESS);
                DoZoneInCombat();
            }

            void DoAction(const int32 action) override
            {
                if (action == ACTION_LIGHT)
                {
                    EntryCheckPredicate pred(NPC_PURIFYING_LIGHT);
                    summons.DoAction(ACTION_LIGHT, pred, 3);
                }
            }

            void JustDied(Unit* /*killer*/) override
            {
                _JustDied();

                Talk(SAY_DEATH);

                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_TWILIGHT_EPIPHANY_DMG);
                DespawnCreatures(NPC_PURIFYING_BLAST);
                DespawnCreatures(NPC_TWILIGHT_BLAST);
            }

            void JustSummoned(Creature* summon) override
            {
                BossAI::JustSummoned(summon);
                //if (summon->GetEntry() == NPC_PURIFYING_LIGHT)
                //    summon->EnterVehicle(me, -1, true);
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                if (me->HealthBelowPct(60) && !bPhase)
                {
                    bPhase = true;

                    events.CancelEvent(EVENT_WAVE_OF_VIRTUE);
                    events.CancelEvent(EVENT_PURIFYING_LIGHT);
                    events.CancelEvent(EVENT_SMITE);
                    events.CancelEvent(EVENT_RIGHTEOUS_SNEAR);
                    events.ScheduleEvent(EVENT_CONTINUE, 6000);
                    DoCast(me, SPELL_TWILIGHT_EPIPHANY);
                    return;
                }

                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_SMITE:
                            DoCastVictim(SPELL_SMITE);
                            events.ScheduleEvent(EVENT_SMITE, urand(5000, 7000));
                            break;
                        case EVENT_PURIFYING_LIGHT:
                            for (uint8 i = 0; i < 3; ++i)
                                me->SummonCreature(NPC_PURIFYING_LIGHT, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ() + 15.0f, me->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 20000);
                            DoCast(me, SPELL_PURIFYING_LIGHT);
                            break;
                        case EVENT_RIGHTEOUS_SNEAR:
                            DoCastAOE(SPELL_RIGHTEOUS_SNEAR_AOE);
                            events.ScheduleEvent(EVENT_RIGHTEOUS_SNEAR, urand(20000, 25000));
                            break;
                        case EVENT_CONTINUE:
                            DoCast(me, SPELL_TRANSFORM, true);
                            events.ScheduleEvent(EVENT_CORRUPTING_TWILIGHT, 10000);
                            events.ScheduleEvent(EVENT_TWILIGHT_BLAST, urand(1000, 2000));
                            events.ScheduleEvent(EVENT_TWILIGHT_SNEAR, urand(5000, 10000));
                            break;
                        case EVENT_TWILIGHT_BLAST:
                            DoCastVictim(SPELL_TWILIGHT_BLAST);
                            events.ScheduleEvent(EVENT_TWILIGHT_BLAST, urand(5000, 7000));
                            break;
                        case EVENT_CORRUPTING_TWILIGHT:
                            for (uint8 i = 0; i < 3; ++i)
                                me->SummonCreature(NPC_CORRUPTING_TWILIGHT, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ() + 15.0f, me->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 20000);
                            DoCast(me, SPELL_CORRUPTING_TWILIGHT);
                            break;
                        case EVENT_TWILIGHT_SNEAR:
                            DoCastAOE(SPELL_TWILIGHT_SNEAR_AOE);
                            events.ScheduleEvent(EVENT_TWILIGHT_SNEAR, urand(20000, 25000));
                            break;
                        default:
                            break;
                    }
                }
                DoMeleeAttackIfReady();
            }

        private:

            bool bPhase;

            void DespawnCreatures(uint32 entry)
            {
                std::list<Creature*> creatures;
                GetCreatureListWithEntryInGrid(creatures, me, entry, 1000.0f);

                if (creatures.empty())
                   return;

                for (std::list<Creature*>::iterator iter = creatures.begin(); iter != creatures.end(); ++iter)
                     (*iter)->DespawnOrUnsummon();
            }
        };   
};

class npc_archbishop_benedictus_purifying_light : public CreatureScript
{
    public:
        npc_archbishop_benedictus_purifying_light() : CreatureScript("npc_archbishop_benedictus_purifying_light") {}

        CreatureAI* GetAI(Creature* creature) const
        {
            return GetInstanceAI<npc_archbishop_benedictus_purifying_lightAI>(creature);
        }

        struct npc_archbishop_benedictus_purifying_lightAI : public Scripted_NoMovementAI
        {
            npc_archbishop_benedictus_purifying_lightAI(Creature* creature) : Scripted_NoMovementAI(creature) {}

            void MovementInform(uint32 type, uint32 data)
            {
                if (data == EVENT_JUMP_2)
                {
                    DoCast(me, ((me->GetEntry() == NPC_PURIFYING_LIGHT) ? SPELL_PURIFYING_BLAST_DMG : SPELL_TWILIGHT_BOLT_DMG), true);
                    me->DespawnOrUnsummon(2000);
                }
            }

            void DoAction(const int32 action)
            {
                if (action == ACTION_LIGHT)
                {
                    if (auto pBenedictus = me->FindNearestCreature(NPC_BENEDICTUS, 200.0f))
                        if (auto pTarget = pBenedictus->AI()->SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                        {
                            float speedZ = 10.0f;
                            float dist = me->GetExactDist2d(pTarget->GetPositionX(), pTarget->GetPositionY());
                            float speedXY = dist * 10.0f / speedZ;
                            me->GetMotionMaster()->MoveJump(pTarget->GetPositionX(), pTarget->GetPositionY(), pTarget->GetPositionZ(), speedXY, speedZ, EVENT_JUMP_2);
                        }                   
                }
                else if (action == ACTION_TWILIGHT)
                {
                    if (auto pBenedictus = me->FindNearestCreature(NPC_BENEDICTUS, 200.0f))
                        if (auto pTarget = pBenedictus->AI()->SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                        {
                            float speedZ = 10.0f;
                            float dist = me->GetExactDist2d(pTarget->GetPositionX(), pTarget->GetPositionY());
                            float speedXY = dist * 10.0f / speedZ;
                            me->GetMotionMaster()->MoveJump(pTarget->GetPositionX(), pTarget->GetPositionY(), pTarget->GetPositionZ(), speedXY, speedZ, EVENT_JUMP_2);
                        } 
                }
            }
        };   
};

class spell_archbishop_benedictus_purifying_light_targeting : public SpellScriptLoader
{
    public:
        spell_archbishop_benedictus_purifying_light_targeting() : SpellScriptLoader("spell_archbishop_benedictus_purifying_light_targeting") {}

        class spell_archbishop_benedictus_purifying_light_targeting_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_archbishop_benedictus_purifying_light_targeting_SpellScript);

            void HandleDummy(SpellEffIndex effIndex)
            {
                if (!GetCaster())
                    return;

                if (Creature* pBenedictus = GetCaster()->ToCreature())
                    pBenedictus->AI()->DoAction((m_scriptSpellId == SPELL_PURIFYING_LIGHT_TARGETING) ? ACTION_LIGHT : ACTION_TWILIGHT);
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_archbishop_benedictus_purifying_light_targeting_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_archbishop_benedictus_purifying_light_targeting_SpellScript();
        }
};

class spell_archbishop_benedictus_righteous_snear_aoe : public SpellScriptLoader
{
    public:
        spell_archbishop_benedictus_righteous_snear_aoe() : SpellScriptLoader("spell_archbishop_benedictus_righteous_snear_aoe") { }

        class spell_archbishop_benedictus_righteous_snear_aoe_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_archbishop_benedictus_righteous_snear_aoe_SpellScript);

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                if (!GetCaster())
                    return;

                if (targets.size() <= 1)
                    return;

                if (auto pBenedictus = GetCaster()->ToCreature())
                    if (auto pTank = pBenedictus->getVictim())
                        targets.remove(pTank);

                if (targets.size() > 1)
                    Trinity::Containers::RandomResizeList(targets, 1);
            }

            void HandleDummy(SpellEffIndex effIndex)
            {
                if (!GetCaster() || !GetHitUnit())
                    return;

                GetCaster()->CastCustomSpell(((m_scriptSpellId == SPELL_RIGHTEOUS_SNEAR_AOE) ? SPELL_RIGHTEOUS_SNEAR_AURA : SPELL_TWILIGHT_SNEAR_AURA), SPELLVALUE_AURA_STACK, 2, GetHitUnit(), true);
            }

            void Register() override
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_archbishop_benedictus_righteous_snear_aoe_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
                OnEffectHitTarget += SpellEffectFn(spell_archbishop_benedictus_righteous_snear_aoe_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_archbishop_benedictus_righteous_snear_aoe_SpellScript();
        }
};

void AddSC_boss_archbishop_benedictus()
{
    new boss_archbishop_benedictus();
    new npc_archbishop_benedictus_purifying_light();
    new spell_archbishop_benedictus_purifying_light_targeting();
    new spell_archbishop_benedictus_righteous_snear_aoe();
}