#include "zulgurub.h"

enum JindoScriptTexts
{
    SAY_AGGRO                   = 0,
    SAY_DEATH                   = 1,
    SAY_KILL_1                  = 2,
    SAY_KILL_2                  = 3,
    SAY_PHASE_2                 = 5,
    SAY_SPAWN                   = 6,
    SOUND_HAKKAR_BREAK_CHAINS   = 24245,
};

enum HakkarScriptTexts
{
    SAY_PHASE   = 2,
    SAY_END_2   = 0,
    SAY_END_1   = 1,
};

enum Spells
{
    SPELL_DRAIN_SPIRIT_ESSENCE  = 97321,
    SPELL_DEADZONE              = 97170,
    SPELL_SHADOWS_OF_HAKKAR     = 97172,
    SPELL_SHADOW_SPIKE_TARGET   = 97158,
    SPELL_SHADOW_SPIKE          = 97160,
    SPELL_HAKKAR_CHAINS         = 97022,
    SPELL_HAKKAR_CHAINS_VISUAL  = 97091,
    SPELL_SPIRIT_WORLD          = 98861,
    SPELL_SPIRIT_WORLD_AURA     = 96689,
    SPELL_SPIRIT_FORM           = 96568,
    SPELL_SPIRIT_FORM_CHAIN     = 96642,
    SPELL_SUMMON_SPIRIT_TARGET  = 97152,
    SPELL_SUMMON_SPIRIT         = 97123,
    SPELL_BODY_SLAM             = 97198,
    SPELL_SPIRIT_WARRIOR_GAZE   = 97597,
    SPELL_FRENZY                = 97088,
    SPELL_SUNDER_RIFT           = 96970,
    SPELL_SUNDER_RIFT_AURA      = 97320,
    SPELL_BRITTLE_BARRIER       = 97417,
};

enum Events
{
    EVENT_DEADZONE          = 1,
    EVENT_SHADOWS_OF_HAKKAR = 2,
    EVENT_PHASE_2           = 3,
    EVENT_SUMMON_SPIRIT     = 4,
    EVENT_BODY_SLAM         = 5,
    EVENT_SHADOW_SPIKE      = 6,
    EVENT_FRENZY            = 7,
    EVENT_END_1             = 8,
    EVENT_END_2             = 9,
    EVENT_END_3             = 10,
    EVENT_END_4             = 11,
    EVENT_SUMMON_GURUBASHI  = 12,
};

enum Adds
{
    NPC_TWISTED_SPIRIT      = 52624,
    NPC_SPIRIT_PORTAL       = 52532,
    NPC_HAKKAR_CHAINS       = 52430,
    NPC_SUNDERED_RIFT       = 52400,
    NPC_BROKEN_GROUND       = 52407,
    NPC_JINDO_THE_BROKEN    = 52154,
    NPC_SPIRIT_OF_HAKKAR    = 52222,
    NPC_GURUBASHI_SPIRIT    = 52730,
};

enum Points
{
    POINT_JINDO = 1,
};

const Position chainPos[3] = 
{
    {-11801.4f, -1678.39f, 53.0471f, 0.0f},
    {-11761.5f, -1649.87f, 52.9657f, 0.0f},
    {-11772.7f, -1676.90f, 53.0477f, 0.0f}
};

const Position hakkarPos = {-11786.5f, -1690.92f, 53.0195f, 1.6057f};
const Position jindoPos = {-11787.84f, -1698.48f, 52.9600f, 1.539f};

const Position enterPos = {-11917.0f, -1230.14f, 92.29f, 4.78f};

class boss_jindo_the_godbreaker : public CreatureScript
{
    public:
        boss_jindo_the_godbreaker() : CreatureScript("boss_jindo_the_godbreaker") { }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new boss_jindo_the_godbreakerAI(pCreature);
        }

        struct boss_jindo_the_godbreakerAI : public BossAI
        {
            boss_jindo_the_godbreakerAI(Creature* pCreature) : BossAI(pCreature, DATA_JINDO)
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
                me->setActive(true);
            }

            bool bTwoPhase;
            uint8 chains;

            void Reset()
            {
                _Reset();
                bTwoPhase = false;
                chains = 0;
                me->SetReactState(REACT_AGGRESSIVE);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                me->SetCanFly(false);
            }

            void EnterCombat(Unit* /*who*/)
            {
                Talk(SAY_AGGRO);
                bTwoPhase = false;
                chains = 0;
                events.RescheduleEvent(EVENT_DEADZONE, 12000);
                events.RescheduleEvent(EVENT_SHADOWS_OF_HAKKAR, 19000);
                instance->SetBossState(DATA_JINDO, IN_PROGRESS);
            }

            void JustDied(Unit* /*killer*/)
            {
                _JustDied();
            }

            void JustSummoned(Creature* summon)
            {
                summons.Summon(summon);
                if (me->isInCombat() && (summon->GetEntry() != NPC_GURUBASHI_SPIRIT))
                    DoZoneInCombat(summon);
            }

            void SummonedCreatureDies(Creature* summon, Unit* killer)
            {
                if (summon->GetEntry() == NPC_HAKKAR_CHAINS)
                {
                    chains++;
                    if (chains == 3)
                    {
                        events.Reset();
                        me->LowerPlayerDamageReq(me->GetHealth());
                        summons.DespawnEntry(NPC_TWISTED_SPIRIT);
                        summons.DespawnEntry(NPC_HAKKAR_CHAINS);
                        std::list<Creature*> creatures;
                        GetCreatureListWithEntryInGrid(creatures, me, NPC_GURUBASHI_SPIRIT, 100.0f);
                        if (!creatures.empty())
                            for (std::list<Creature*>::iterator iter = creatures.begin(); iter != creatures.end(); ++iter)
                                (*iter)->DespawnOrUnsummon();

                        if (Player* realKiller = killer->GetCharmerOrOwnerPlayerOrPlayerItself())
                            if (Creature* pHakkar = me->FindNearestCreature(NPC_SPIRIT_OF_HAKKAR, 100.0f))
                                realKiller->ToPlayer()->SendSoundToAll(SOUND_HAKKAR_BREAK_CHAINS, pHakkar->GetGUID());
                        events.RescheduleEvent(EVENT_END_1, 4000);                            
                    }
                }    
            }

            void MovementInform(uint32 type, uint32 id)
            {
                if (type == POINT_MOTION_TYPE)
                {
                    if (id == POINT_JINDO)
                    {
                        DoCast(me, SPELL_SPIRIT_WORLD, true);
                        events.RescheduleEvent(EVENT_PHASE_2, 4000);
                        me->SetFacingTo(jindoPos.GetOrientation());
                        for (uint8 i = 0; i < 3; ++i)
                            me->SummonCreature(NPC_HAKKAR_CHAINS, chainPos[i], TEMPSUMMON_DEAD_DESPAWN);
                    
                    }
                }
            }

            void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
            {
                if (me->HealthBelowPct(70) && !bTwoPhase)
                {
                    bTwoPhase = true;
                    me->AttackStop();
                    me->InterruptNonMeleeSpells(false);
                    me->SetReactState(REACT_PASSIVE);
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                    events.Reset();
                    Talk(SAY_PHASE_2);

                    if (Creature* pHakkar = me->SummonCreature(NPC_SPIRIT_OF_HAKKAR, hakkarPos))
                    {
                        pHakkar->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                        pHakkar->CastSpell(pHakkar, SPELL_SPIRIT_FORM, true);
                        pHakkar->CastSpell(pHakkar, SPELL_HAKKAR_CHAINS_VISUAL, true);
                    }
                    me->GetMotionMaster()->MovePoint(POINT_JINDO, jindoPos);
                }
            }

            void UpdateAI(uint32 diff)
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
                        case EVENT_DEADZONE:
                            DoCast(me, SPELL_DEADZONE);
                            events.RescheduleEvent(EVENT_DEADZONE, 19000);
                            break;
                        case EVENT_SHADOWS_OF_HAKKAR:
                            DoCast(me, SPELL_SHADOWS_OF_HAKKAR);
                            events.RescheduleEvent(EVENT_SHADOWS_OF_HAKKAR, 18000);
                            break;
                        case EVENT_PHASE_2:
                            if (Creature* pHakkar = me->FindNearestCreature(NPC_SPIRIT_OF_HAKKAR, 100.0f))                               
                            {
                                pHakkar->AI()->Talk(SAY_PHASE);
                                for (SummonList::const_iterator itr = summons.begin(); itr != summons.end(); ++itr)
                                    if (Creature* pSummon = Unit::GetCreature(*me, (*itr))) 
                                        if (pSummon->GetEntry() == NPC_HAKKAR_CHAINS)
                                            pSummon->CastSpell(pHakkar, SPELL_HAKKAR_CHAINS);
                            }
                            me->SetCanFly(true);
                            me->GetMotionMaster()->MoveJump(jindoPos.GetPositionX(), jindoPos.GetPositionY(), jindoPos.GetPositionZ() + 15.0f, 20.0f, 40.0f); 
                            events.RescheduleEvent(EVENT_SHADOW_SPIKE, 3000);
                            events.RescheduleEvent(EVENT_SUMMON_SPIRIT, 4000);
                            break;
                        case EVENT_SHADOW_SPIKE:
                            DoCast(me, SPELL_SHADOW_SPIKE_TARGET);
                            events.RescheduleEvent(EVENT_SHADOW_SPIKE, 5000);
                            break;
                        case EVENT_SUMMON_SPIRIT:
                            DoCast(me, SPELL_SUMMON_SPIRIT_TARGET);
                            events.RescheduleEvent(EVENT_SUMMON_SPIRIT, 6000);
                            break;
                        case EVENT_END_1:
                        {
                            if (Creature* pHakkar = me->FindNearestCreature(NPC_SPIRIT_OF_HAKKAR, 100.0f))
                                pHakkar->AI()->Talk(SAY_END_1);
                            events.RescheduleEvent(EVENT_END_2, 20000);
                            break;
                        }
                        case EVENT_END_2:
                            Talk(SAY_DEATH);
                            me->SetCanFly(false);
                            me->GetMotionMaster()->MoveJump(jindoPos.GetPositionX(), jindoPos.GetPositionY(), jindoPos.GetPositionZ(), 20.0f, 40.0f); 
                            events.RescheduleEvent(EVENT_END_3, 5000);
                            break;
                        case EVENT_END_3:
                            if (Creature* pHakkar = me->FindNearestCreature(NPC_SPIRIT_OF_HAKKAR, 100.0f))
                                pHakkar->AI()->Talk(SAY_END_2);
                            events.RescheduleEvent(EVENT_END_4, 8000);
                            break;
                        case EVENT_END_4:
                            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                            if (Unit* pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                                pTarget->Kill(me);
                            me->RemoveAllAuras();
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }
        };
};

class npc_jindo_gurubashi_spirit : public CreatureScript
{
    public:

        npc_jindo_gurubashi_spirit() : CreatureScript("npc_jindo_gurubashi_spirit") {}
        
        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_jindo_gurubashi_spiritAI(pCreature);
        }

        struct npc_jindo_gurubashi_spiritAI : public ScriptedAI
        {
            npc_jindo_gurubashi_spiritAI(Creature* pCreature) : ScriptedAI(pCreature) 
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

            EventMap events;

            void Reset()
            {
                events.Reset();
            }

            void EnterCombat(Unit* who)
            {
                events.RescheduleEvent(EVENT_BODY_SLAM, 12000);
                events.RescheduleEvent(EVENT_FRENZY, 7000);
                DoZoneInCombat(me, 100.0f);
            }

            void JustSummoned(Creature* summon)
            {
                if (Creature* pJindo = me->FindNearestCreature(NPC_JINDO, 100.0f))
                    static_cast<boss_jindo_the_godbreaker::boss_jindo_the_godbreakerAI*>(pJindo->GetAI())->JustSummoned(summon);
            }

            void MovementInform(uint32 type, uint32 id)
            {
                if (id == 97198)
                    if (Creature* pChain = me->FindNearestCreature(NPC_HAKKAR_CHAINS, 5.0f))
                        pChain->RemoveAurasDueToSpell(SPELL_BRITTLE_BARRIER);
            }

            void UpdateAI(uint32 diff)
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
                        case EVENT_FRENZY:
                            DoCast(me, SPELL_FRENZY);
                            events.RescheduleEvent(EVENT_FRENZY, 10000);
                            break;
                        case EVENT_BODY_SLAM:
                            DoCast(me, SPELL_SPIRIT_WARRIOR_GAZE);
                            events.RescheduleEvent(EVENT_BODY_SLAM, 25000);
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }
        };
};

class npc_jindo_spirit_of_hakkar : public CreatureScript
{
    public:

        npc_jindo_spirit_of_hakkar() : CreatureScript("npc_jindo_spirit_of_hakkar") {}
        
        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_jindo_spirit_of_hakkarAI(pCreature);
        }

        struct npc_jindo_spirit_of_hakkarAI : public Scripted_NoMovementAI
        {
            npc_jindo_spirit_of_hakkarAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature) 
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
                me->SetReactState(REACT_PASSIVE);
            }

            void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
            {
                damage = 0;
            }
        };
};

class npc_jindo_chains_of_hakkar : public CreatureScript
{
    public:

        npc_jindo_chains_of_hakkar() : CreatureScript("npc_jindo_chains_of_hakkar") {}
        
        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_jindo_chains_of_hakkarAI(pCreature);
        }

        struct npc_jindo_chains_of_hakkarAI : public Scripted_NoMovementAI
        {
            npc_jindo_chains_of_hakkarAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature) 
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
                me->SetReactState(REACT_PASSIVE);
            }

            void EnterCombat(Unit* who)
            {
                DoZoneInCombat(me, 300.0f);
            }
        };
};

class npc_jindo_spirit_portal : public CreatureScript
{
    public:

        npc_jindo_spirit_portal() : CreatureScript("npc_jindo_spirit_portal") {}
        
        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_jindo_spirit_portalAI(pCreature);
        }

        struct npc_jindo_spirit_portalAI : public Scripted_NoMovementAI
        {
            npc_jindo_spirit_portalAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature) 
            {
            }

            void JustSummoned(Creature* summon)
            {
                if (Creature* pJindo = me->FindNearestCreature(NPC_JINDO, 300.0f))
                    static_cast<boss_jindo_the_godbreaker::boss_jindo_the_godbreakerAI*>(pJindo->GetAI())->JustSummoned(summon);
            }
        };
};

class spell_jindo_shadow_spike_target : public SpellScriptLoader
{
    public:
        spell_jindo_shadow_spike_target() : SpellScriptLoader("spell_jindo_shadow_spike_target") { }


        class spell_jindo_shadow_spike_target_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_jindo_shadow_spike_target_SpellScript);

            void HandleDummy(SpellEffIndex effIndex)
            {
                if (!GetCaster() || !GetHitUnit())
                    return;

                GetCaster()->CastSpell(GetHitUnit(), SPELL_SHADOW_SPIKE, true);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_jindo_shadow_spike_target_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_jindo_shadow_spike_target_SpellScript();
        }
};

class SpiritPortalCheck
{
    public:
        SpiritPortalCheck(uint32 entry) {i_entry = entry;}
        bool operator()(WorldObject* obj) const
        {
            if (!obj->ToCreature())
                return true;
            return ((obj->ToCreature()->HasUnitState(UNIT_STATE_CASTING)) || (obj->ToCreature()->GetEntry() != i_entry) || !obj->ToCreature()->isAlive());
        }
        uint32 i_entry;
};

class spell_jindo_summon_spirit_target : public SpellScriptLoader
{
    public:
        spell_jindo_summon_spirit_target() : SpellScriptLoader("spell_jindo_summon_spirit_target") { }


        class spell_jindo_summon_spirit_target_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_jindo_summon_spirit_target_SpellScript);

            void FilterTargets(std::list<WorldObject*>& targets)
            { 
                targets.remove_if(SpiritPortalCheck(NPC_SPIRIT_PORTAL));
                if (targets.size() > 1)
                    Trinity::Containers::RandomResizeList(targets, 1);
            }

            void HandleDummy(SpellEffIndex effIndex)
            {
                if (!GetCaster() || !GetHitUnit())
                    return;

                GetHitUnit()->CastSpell(GetHitUnit(), SPELL_SUMMON_SPIRIT);
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_jindo_summon_spirit_target_SpellScript::FilterTargets, EFFECT_0,TARGET_UNIT_SRC_AREA_ENTRY);
                OnEffectHitTarget += SpellEffectFn(spell_jindo_summon_spirit_target_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_jindo_summon_spirit_target_SpellScript();
        }
};

class spell_jindo_spirit_warrior_gaze_target : public SpellScriptLoader
{
    public:
        spell_jindo_spirit_warrior_gaze_target() : SpellScriptLoader("spell_jindo_spirit_warrior_gaze_target") { }

        class spell_jindo_spirit_warrior_gaze_target_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_jindo_spirit_warrior_gaze_target_SpellScript);

            void HandleApplyAura(SpellEffIndex effIndex)
            {
                if (!GetCaster() || !GetHitUnit())
                    return;

                GetCaster()->CastSpell(GetHitUnit(), SPELL_BODY_SLAM);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_jindo_spirit_warrior_gaze_target_SpellScript::HandleApplyAura, EFFECT_0, SPELL_EFFECT_APPLY_AURA);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_jindo_spirit_warrior_gaze_target_SpellScript();
        }
};


void AddSC_boss_jindo_the_godbreaker()
{
    new boss_jindo_the_godbreaker();
    new npc_jindo_gurubashi_spirit();
    new npc_jindo_spirit_of_hakkar();
    new npc_jindo_chains_of_hakkar();
    new npc_jindo_spirit_portal();
    new spell_jindo_shadow_spike_target();
    new spell_jindo_summon_spirit_target();
    new spell_jindo_spirit_warrior_gaze_target();
}
