#include "Spell.h"
#include "zulgurub.h"

enum ScriptTexts
{
    SAY_AGGRO       = 0,
    SAY_DEATH       = 1,
    SAY_KILL_FERAL  = 2,
    SAY_KILL        = 3,
    SAY_SPELL       = 4,
    SAY_TRANSFORM_1 = 5,
    SAY_TRANSFORM_2 = 6,
};

enum Spells
{
    SPELL_SHADOW_BOLT           = 96516,
    SPELL_SHADOW_BOLT_H         = 96956,
    SPELL_WAVE_OF_AGONY_AOE     = 98269, // select target
    SPELL_WAVE_OF_AGONY_END     = 96461, // summon stalker
    SPELL_WAVE_OF_AGONY_START   = 96457, // summon wave stalker
    SPELL_WAVE_OF_AGONY_DMG     = 96460,
    SPELL_WAVE_OF_AGONY_VISUAL  = 96532,
    SPELL_WAVE_OF_AGONY_TRAJ    = 96542,
    SPELL_TEARS_OF_BLOOD        = 96438,
    SPELL_TEARS_OF_BLOOD_AURA   = 96935,
    SPELL_WAIL_OF_SORROW        = 96909,
    SPELL_LASH_OF_ANGUISH       = 96423,
    SPELL_RAGE_OF_THE_ANCIENTS  = 96896,
    SPELL_VENGEFUL_SMASH        = 96593,
    SPELL_CAMOUFLAGE            = 96594,
    SPELL_RAVAGE                = 96592,
    SPELL_PRIMAL_BLESSING       = 96559,
    SPELL_CAVE_IN               = 97380,
    SPELL_CAVE_IN_DUMMY         = 96935,
    SPELL_GAPING_WOUND          = 97355,
    SPELL_DARK_SLUMBER          = 96446,

    // achievement
    SPELL_CLEAR_ACHIEVEMENT     = 98840,
    SPELL_CAT_FED               = 98258,
    SPELL_BLOOD_FRENZY          = 98239,
    SPELL_CREATE_RAT_COVER      = 98177,
    SPELL_CREATE_RAT            = 98178,
    SPELL_THROW_RAT             = 98181,
    SPELL_HAS_RAT               = 98208,
    SPELL_RAT_LANDS             = 98216,
    SPELL_RAT_LURE              = 98238,
    SPELL_POUNCE_RAT            = 98241,
    SPELL_RAT_MUNCH             = 98242,
};

enum Events
{
    EVENT_SHADOW_BOLT       = 1,
    EVENT_WAVE_OF_AGONY     = 2,
    EVENT_TEARS_OF_BLOOD    = 3,
    EVENT_WAIL_OF_SORROW    = 4,
    EVENT_LASH_OF_ANGUISH   = 5,
    EVENT_CAMOUFLAGE        = 6,
    EVENT_RAVAGE            = 7,
    EVENT_GAPING_WOUND      = 8,
    EVENT_CONTINUE          = 9,
    EVENT_VENGEFUL_SMASH    = 10,
    EVENT_PRIMAL_BLESSING   = 11,
};

enum Adds
{
    NPC_PRIDE_OF_BETHEKK    = 52061,
    NPC_WAVE_OF_AGONY       = 52147,
    NPC_WAVE_OF_AGONY_END   = 52160,
    NPC_CAVE_IN_STALKER     = 52387,
    NPC_TEMPLE_RAT          = 53108,
};

const Position pridePos[16] = 
{
    {-11517.2f, -1646.82f, 44.4849f, 3.87463f},
    {-11519.2f, -1605.37f, 44.4849f, 3.56047f},
    {-11507.0f, -1644.55f, 44.4849f, 4.7822f},
    {-11519.7f, -1609.0f, 44.4849f, 2.26893f},
    {-11505.6f, -1607.56f, 44.4849f, 2.26893f},
    {-11504.6f, -1603.33f, 44.4849f, 3.87463f},
    {-11508.3f, -1607.37f, 44.4849f, 0.977384f},
    {-11508.7f, -1603.38f, 44.4849f, 5.41052f},
    {-11504.3f, -1645.56f, 44.4849f, 4.66003f},
    {-11523.2f, -1605.96f, 44.4849f, 5.41052f},
    {-11504.1f, -1650.26f, 44.4849f, 2.26893f},
    {-11506.6f, -1651.04f, 44.4849f, 1.6057f},
    {-11523.2f, -1609.31f, 44.4849f, 0.977384f},
    {-11521.8f, -1651.58f, 44.4849f, 0.977384f},
    {-11520.6f, -1646.0f, 44.4849f, 4.95674f},
    {-11518.1f, -1651.48f, 44.4849f, 2.26893f}
};

const Position cavePos[4] = 
{
    {-11506.63f, -1605.05f, 44.41f, 0.0f},
    {-11506.26f, -1643.40f, 44.41f, 0.0f},
    {-11520.27f, -1644.65f, 44.41f, 0.0f},
    {-11522.52f, -1610.81f, 44.41f, 0.0f}
};



class boss_kilnara : public CreatureScript
{
    public:
        boss_kilnara() : CreatureScript("boss_kilnara") { }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new boss_kilnaraAI(pCreature);
        }

        struct boss_kilnaraAI : public BossAI
        {
            boss_kilnaraAI(Creature* creature) : BossAI(creature, DATA_KILNARA)
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
            }

            bool bTwoPhase;
            uint8 rats;

            void Reset()
            {
                _Reset();

                for (uint8 i = 0; i < 16; ++i)
                    if (Creature* pPride = me->SummonCreature(NPC_PRIDE_OF_BETHEKK, pridePos[i]))
                    {
                        pPride->SetReactState(REACT_DEFENSIVE);
                        pPride->CastSpell(pPride, SPELL_DARK_SLUMBER, true);
                    }

                bTwoPhase = false;
                rats = 0;
                me->SetReactState(REACT_AGGRESSIVE);
            }

            void EnterCombat(Unit* /*who*/)
            {
                Talk(SAY_AGGRO);
                bTwoPhase = false;
                rats = 0;
                events.RescheduleEvent(EVENT_SHADOW_BOLT, 1000);
                events.RescheduleEvent(EVENT_WAVE_OF_AGONY, urand(18000, 25000));
                events.RescheduleEvent(EVENT_LASH_OF_ANGUISH, 10000);
                events.RescheduleEvent(EVENT_WAIL_OF_SORROW, urand(15000, 20000));
                events.RescheduleEvent(EVENT_TEARS_OF_BLOOD, urand(12000, 15000));
                instance->DoResetAchievementCriteria(CRITERIA_TYPE_BE_SPELL_TARGET, CRITERIA_CONDITION_NO_SPELL_HIT, SPELL_CLEAR_ACHIEVEMENT);
                instance->DoResetAchievementCriteria(CRITERIA_TYPE_KILL_CREATURE, CRITERIA_CONDITION_NO_SPELL_HIT, SPELL_CLEAR_ACHIEVEMENT);
                instance->SetBossState(DATA_KILNARA, IN_PROGRESS);
            }

            void JustDied(Unit* /*killer*/)
            {
                _JustDied();
                Talk(SAY_DEATH);
            }

            void JustSummoned(Creature* summon)
            {
                BossAI::JustSummoned(summon);
                if (summon->GetEntry() == NPC_WAVE_OF_AGONY_END)
                {
                    Talk(SAY_SPELL);
                    DoCast(summon, SPELL_WAVE_OF_AGONY_START, true);
                }
            }

            void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
            {
                if (me->GetGUID() == attacker->GetGUID())
                    damage = 0;
            }

            void KilledUnit(Unit* who)
            {
                if (who->GetTypeId() == TYPEID_PLAYER)
                    Talk(bTwoPhase? SAY_KILL_FERAL: SAY_KILL);
            }

            void SpellHit(Unit* caster, SpellInfo const* spellInfo)
            {
                if (spellInfo->HasEffect(SPELL_EFFECT_INTERRUPT_CAST))
                {
                    if (Spell const* spell = me->GetCurrentSpell(CURRENT_GENERIC_SPELL))
                        if (spell->m_spellInfo->Id == SPELL_SHADOW_BOLT ||
                            spell->m_spellInfo->Id == SPELL_SHADOW_BOLT_H)
                            me->InterruptSpell(CURRENT_GENERIC_SPELL);

                    me->RemoveAurasDueToSpell(SPELL_TEARS_OF_BLOOD_AURA);
                    me->RemoveAurasDueToSpell(SPELL_TEARS_OF_BLOOD);
                }
            }

            void UpdateAI(uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HealthBelowPct(50) && !bTwoPhase)
                {
                    bTwoPhase = true;
                    me->InterruptNonMeleeSpells(false);
                    events.Reset();
                    me->AttackStop();
                    me->SetReactState(REACT_PASSIVE);
                    Talk(SAY_TRANSFORM_1);
                    DoCast(me, SPELL_CAVE_IN);
                    for (uint8 i = 0; i < 4; ++i)
                        if (Creature* pCave = me->SummonCreature(NPC_CAVE_IN_STALKER, cavePos[i], TEMPSUMMON_TIMED_DESPAWN, 5000))
                            pCave->CastSpell(pCave, SPELL_CAVE_IN_DUMMY);
                    summons.DoZoneInCombat(NPC_PRIDE_OF_BETHEKK);
                    events.RescheduleEvent(EVENT_PRIMAL_BLESSING, 6000);
                    events.RescheduleEvent(EVENT_CONTINUE, 7000);
                    return;
                }

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;
            
                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_SHADOW_BOLT:
                            DoCastVictim(SPELL_SHADOW_BOLT);
                            events.RescheduleEvent(EVENT_SHADOW_BOLT, urand(7000, 8000));
                            break;
                        case EVENT_WAIL_OF_SORROW:
                            DoCast(me, SPELL_WAIL_OF_SORROW);
                            events.RescheduleEvent(EVENT_WAIL_OF_SORROW, urand(15000, 20000));
                            break;
                        case EVENT_LASH_OF_ANGUISH:
                            if (Unit* pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                                DoCast(pTarget, SPELL_LASH_OF_ANGUISH);
                            events.RescheduleEvent(EVENT_LASH_OF_ANGUISH, urand(15000, 20000));
                            break;
                        case EVENT_TEARS_OF_BLOOD:
                            DoCast(me, SPELL_TEARS_OF_BLOOD);
                            events.RescheduleEvent(EVENT_TEARS_OF_BLOOD, urand(30000, 35000));
                            break;
                        case EVENT_WAVE_OF_AGONY:
                            DoCast(me, SPELL_WAVE_OF_AGONY_AOE);
                            events.RescheduleEvent(EVENT_WAVE_OF_AGONY, urand(30000, 35000));
                            break;
                        case EVENT_PRIMAL_BLESSING:
                            Talk(SAY_TRANSFORM_2);
                            DoCast(me, SPELL_PRIMAL_BLESSING, true);
                            DoCast(me, SPELL_RAGE_OF_THE_ANCIENTS, true);
                            break;
                        case EVENT_CONTINUE:
                            me->SetReactState(REACT_AGGRESSIVE);
                            AttackStart(me->getVictim());
                            events.RescheduleEvent(EVENT_VENGEFUL_SMASH, urand(4000, 10000));
                            events.RescheduleEvent(EVENT_RAVAGE, urand(2000, 6000));
                            break;
                        case EVENT_VENGEFUL_SMASH:
                            DoCast(me, SPELL_VENGEFUL_SMASH);
                            events.RescheduleEvent(EVENT_VENGEFUL_SMASH, urand(10000, 15000));
                            break;
                        case EVENT_RAVAGE:
                            DoCastVictim(SPELL_RAVAGE);
                            events.RescheduleEvent(EVENT_RAVAGE, urand(9000, 12000));
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }
        };
};

class npc_kilnara_pride_of_bethekk : public CreatureScript
{
    public:

        npc_kilnara_pride_of_bethekk() : CreatureScript("npc_kilnara_pride_of_bethekk") {}
        
        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_kilnara_pride_of_bethekkAI(pCreature);
        }

        struct npc_kilnara_pride_of_bethekkAI : public ScriptedAI
        {
            npc_kilnara_pride_of_bethekkAI(Creature* pCreature) : ScriptedAI(pCreature) 
            {
            }

            uint32 uiGapingWound;

            void Reset()
            {
                uiGapingWound = urand(5000, 15000);
            }

            void UpdateAI(uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                if (uiGapingWound <= diff)
                {
                    uiGapingWound = urand(5000, 15000);
                    if (Unit* pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                        DoCast(pTarget, SPELL_GAPING_WOUND);
                }
                else
                    uiGapingWound -= diff;

                DoMeleeAttackIfReady();
            }

        };
};

class npc_kilnara_wave_of_agony : public CreatureScript
{
    public:

        npc_kilnara_wave_of_agony() : CreatureScript("npc_kilnara_wave_of_agony") {}
        
        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_kilnara_wave_of_agonyAI(pCreature);
        }

        struct npc_kilnara_wave_of_agonyAI : public Scripted_NoMovementAI
        {
            npc_kilnara_wave_of_agonyAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature) 
            {
                me->SetReactState(REACT_PASSIVE);
            }

            void Reset()
            {
            }
        };
};

class npc_kilnara_temple_rat : public CreatureScript
{
    public:

        npc_kilnara_temple_rat() : CreatureScript("npc_kilnara_temple_rat") {}
        
        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_kilnara_temple_ratAI(pCreature);
        }

        struct npc_kilnara_temple_ratAI : public ScriptedAI
        {
            npc_kilnara_temple_ratAI(Creature* pCreature) : ScriptedAI(pCreature) 
            {
            }

            void IsSummonedBy(Unit* summoner)
            {
                if (me->GetEntry() == NPC_TEMPLE_RAT)
                    DoCast(me, SPELL_RAT_LURE, true);
            }

            void OnSpellClick(Unit* clicker)
            {
                me->DespawnOrUnsummon();
            }
        };
};

class spell_kilnara_wave_of_agony_target : public SpellScriptLoader
{
    public:
        spell_kilnara_wave_of_agony_target() : SpellScriptLoader("spell_kilnara_wave_of_agony_target") { }


        class spell_kilnara_wave_of_agony_target_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_kilnara_wave_of_agony_target_SpellScript);


            void HandleScript(SpellEffIndex /*effIndex*/)
            { 
                if (!GetCaster() || !GetHitUnit())
                    return;

                GetCaster()->CastSpell(GetHitUnit(), SPELL_WAVE_OF_AGONY_END, true);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_kilnara_wave_of_agony_target_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_kilnara_wave_of_agony_target_SpellScript();
        }
};

class spell_kilnara_wave_of_agony_start : public SpellScriptLoader
{
    public:
        spell_kilnara_wave_of_agony_start() : SpellScriptLoader("spell_kilnara_wave_of_agony_start") { }

        class spell_kilnara_wave_of_agony_start_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_kilnara_wave_of_agony_start_AuraScript);

            void PeriodicTick(AuraEffect const* aurEff)
            {
                if (!GetUnitOwner())
                    return;

                if (pStart = GetUnitOwner()->FindNearestCreature(NPC_WAVE_OF_AGONY, 300.0f))
                {
                    pStart->CastSpell(GetUnitOwner(), SPELL_WAVE_OF_AGONY_TRAJ, true);
                    pStart->DespawnOrUnsummon(500);
                }
                if (GetUnitOwner()->ToCreature())
                    GetUnitOwner()->ToCreature()->DespawnOrUnsummon(500);
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_kilnara_wave_of_agony_start_AuraScript::PeriodicTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
            }

        private:
            Creature* pStart;
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_kilnara_wave_of_agony_start_AuraScript();
        }
};

class PrideCheck
{
    public:
        PrideCheck() {}
        bool operator()(WorldObject* obj) const
        {
            if (!obj->ToCreature())
                return true;
            return ((obj->ToCreature()->GetEntry() != NPC_PRIDE_OF_BETHEKK) || !obj->ToCreature()->isAlive() || obj->ToCreature()->HasAura(SPELL_DARK_SLUMBER) || obj->ToCreature()->HasAura(SPELL_BLOOD_FRENZY));
        }
};

class spell_kilnara_rat_lure : public SpellScriptLoader
{
    public:
        spell_kilnara_rat_lure() : SpellScriptLoader("spell_kilnara_rat_lure") { }


        class spell_kilnara_rat_lure_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_kilnara_rat_lure_SpellScript);

            void FilterTargets(std::list<WorldObject*>& targets)
            { 
                targets.remove_if(PrideCheck());
                if (targets.size() > 1)
                    Trinity::Containers::RandomResizeList(targets, 1);
            }

            void HandleScript(SpellEffIndex effIndex)
            {
                if (!GetCaster() || !GetHitUnit())
                    return;

                GetHitUnit()->CastSpell(GetHitUnit(), SPELL_BLOOD_FRENZY, true);
                GetHitUnit()->CastSpell(GetCaster(), SPELL_POUNCE_RAT, true);
                if (InstanceScript* instance = GetHitUnit()->GetInstanceScript())
                    instance->DoUpdateAchievementCriteria(CRITERIA_TYPE_BE_SPELL_TARGET, SPELL_CAT_FED);
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_kilnara_rat_lure_SpellScript::FilterTargets, EFFECT_0,TARGET_UNIT_SRC_AREA_ENTRY);
                OnEffectHitTarget += SpellEffectFn(spell_kilnara_rat_lure_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_kilnara_rat_lure_SpellScript();
        }
};

void AddSC_boss_kilnara()
{
    new boss_kilnara();
    new npc_kilnara_pride_of_bethekk();
    new npc_kilnara_wave_of_agony();
    new npc_kilnara_temple_rat();
    new spell_kilnara_wave_of_agony_target();
    new spell_kilnara_wave_of_agony_start();
    new spell_kilnara_rat_lure();
}
