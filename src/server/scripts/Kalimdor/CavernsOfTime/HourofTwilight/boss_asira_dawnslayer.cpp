#include "hour_of_twilight.h"

enum ScriptTexts
{
    SAY_AGGRO   = 0,
    SAY_DEATH   = 1,
    SAY_EVENT_1 = 2,
    SAY_EVENT_2 = 3,
    SAY_EVENT_3 = 4,
    SAY_KILL    = 5,
    SAY_SPELL   = 6
};

enum Spells
{
    SPELL_MARK_OF_SILENCE           = 102726,
    SPELL_THROW_KNIFE               = 103597,
    SPELL_SILENCE                   = 103587,
    SPELL_CHOKING_SMOKE_BOMB        = 103558,
    SPELL_CHOKING_SMOKE_BOMB_DMG    = 103790,
    SPELL_BLADE_BARRIER             = 103419,
    SPELL_LESSER_BLADE_BARRIER      = 103562
};

enum Events
{
    EVENT_MARK_OF_SILENCE       = 1,
    EVENT_CHOKING_SMOKE_BOMB    = 2
};

class boss_asira_dawnslayer : public CreatureScript
{
    public:
        boss_asira_dawnslayer() : CreatureScript("boss_asira_dawnslayer") {}

        CreatureAI* GetAI(Creature* creature) const
        {
            return GetInstanceAI<boss_asira_dawnslayerAI>(creature);
        }

        struct boss_asira_dawnslayerAI : public BossAI
        {
            boss_asira_dawnslayerAI(Creature* creature) : BossAI(creature, DATA_ASIRA)
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
                bBarrier = false;
            }

            void Reset() override
            {
                _Reset();
                bBarrier = false;
            }

            void EnterCombat(Unit* /*who*/) override
            {
                Talk(SAY_AGGRO);

                bBarrier = false;

                events.ScheduleEvent(EVENT_MARK_OF_SILENCE, urand(2000, 3000));
                events.ScheduleEvent(EVENT_CHOKING_SMOKE_BOMB, urand(10000, 12000));

                instance->SetBossState(DATA_ASIRA, IN_PROGRESS);
                DoZoneInCombat();
            }

            void KilledUnit(Unit* who) override
            {
                if (who && who->IsPlayer())
                    Talk(SAY_KILL);
            }

            void JustDied(Unit* /*killer*/) override
            {
                _JustDied();
                Talk(SAY_DEATH);

                me->SummonCreature(NPC_LIFE_WARDEN_2, drakePos);
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                if (me->HealthBelowPct(30) && !bBarrier)
                {
                    bBarrier = true;
                    DoCast(me, SPELL_BLADE_BARRIER);
                    return;
                }

                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_MARK_OF_SILENCE:
                            DoCastAOE(SPELL_MARK_OF_SILENCE);
                            events.ScheduleEvent(EVENT_MARK_OF_SILENCE, urand(21000, 23000));
                            break;
                        case EVENT_CHOKING_SMOKE_BOMB:
                            Talk(SAY_SPELL);
                            DoCast(me, SPELL_CHOKING_SMOKE_BOMB);
                            events.ScheduleEvent(EVENT_CHOKING_SMOKE_BOMB, urand(19000, 21000));
                            break;
                        default:
                            break;
                    }
                }
                DoMeleeAttackIfReady();
            }

        private:
            bool bBarrier;
        };   
};

class spell_asira_dawnslayer_blade_barrier : public SpellScriptLoader
{
    public:
        spell_asira_dawnslayer_blade_barrier() : SpellScriptLoader("spell_asira_dawnslayer_blade_barrier") {}

        class spell_asira_dawnslayer_blade_barrier_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_asira_dawnslayer_blade_barrier_AuraScript);

            void CalculateAmount(AuraEffect const* /*aurEff*/, float & amount, bool& /*canBeRecalculated*/)
            {
                amount = -1;
            }

            void Absorb(AuraEffect* aurEff, DamageInfo & dmgInfo, float & absorbAmount)
            {
                if (dmgInfo.GetDamage() < (uint32)GetSpellInfo()->Effects[EFFECT_0]->BasePoints)
                    absorbAmount = dmgInfo.GetDamage() - 1;
                else
                    GetAura()->Remove();
            }

            void HandleAfterRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (GetCaster())
                    GetCaster()->CastSpell(GetCaster(), SPELL_LESSER_BLADE_BARRIER, true);
            }

            void Register() override
            {
                 DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_asira_dawnslayer_blade_barrier_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
                 OnEffectAbsorb += AuraEffectAbsorbFn(spell_asira_dawnslayer_blade_barrier_AuraScript::Absorb, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
                 if (m_scriptSpellId == SPELL_BLADE_BARRIER)
                     AfterEffectRemove += AuraEffectRemoveFn(spell_asira_dawnslayer_blade_barrier_AuraScript::HandleAfterRemove, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_asira_dawnslayer_blade_barrier_AuraScript();
        }
};

class spell_asira_dawnslayer_throw_knife : public SpellScriptLoader
{ 
    public:
        spell_asira_dawnslayer_throw_knife() : SpellScriptLoader("spell_asira_dawnslayer_throw_knife") { }

        class spell_asira_dawnslayer_throw_knife_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_asira_dawnslayer_throw_knife_SpellScript);

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                if (!GetCaster())
                    return;

                if (targets.size() <= 1)
                    return;

                Unit* pPlayer = GetExplTargetUnit();
                if (!pPlayer)
                    return;

                WorldObject* objTarget = NULL;
                for (std::list<WorldObject*>::const_iterator itr = targets.begin(); itr != targets.end(); ++itr)
                    if ((*itr)->IsInBetween(GetCaster(), pPlayer, 1.0f))
                        if (!objTarget || (GetCaster()->GetDistance(objTarget) > GetCaster()->GetDistance((*itr))))
                            objTarget = (*itr);

                if (!objTarget)
                    objTarget = pPlayer;

                targets.clear();
                targets.push_back(objTarget);
            }

            void HandleDamage(SpellEffIndex /*effIndex*/)
            {
                if (!GetCaster() || !GetHitUnit())
                    return;

                if (GetHitUnit()->HasAura(SPELL_MARK_OF_SILENCE))
                    GetCaster()->CastSpell(GetHitUnit(), SPELL_SILENCE, true);
            }
        
            void Register() override
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_asira_dawnslayer_throw_knife_SpellScript::FilterTargets, EFFECT_1, TARGET_UNIT_CONE_ENEMY_24);
                OnEffectHitTarget += SpellEffectFn(spell_asira_dawnslayer_throw_knife_SpellScript::HandleDamage, EFFECT_1, SPELL_EFFECT_SCHOOL_DAMAGE);
            }
        };
       
        SpellScript* GetSpellScript() const
        {
            return new spell_asira_dawnslayer_throw_knife_SpellScript();
        }
};

class spell_asira_dawnslayer_mark_of_silence : public SpellScriptLoader
{ 
    public:
        spell_asira_dawnslayer_mark_of_silence() : SpellScriptLoader("spell_asira_dawnslayer_mark_of_silence") {}

        class spell_asira_dawnslayer_mark_of_silence_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_asira_dawnslayer_mark_of_silence_SpellScript);

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                if (!GetCaster())
                    return;

                if (targets.empty())
                    return;

                targets.remove_if(CastersCheck());
            }

            void Register() override
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_asira_dawnslayer_mark_of_silence_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
            }

        private:

            class CastersCheck
            {
                public:
                    CastersCheck() {}
            
                    bool operator()(WorldObject* unit)
                    {
                        if (unit->GetTypeId() != TYPEID_PLAYER)
                            return true;
                        
                        switch (unit->ToPlayer()->getClass())
                        {
                            case CLASS_WARRIOR:
                            case CLASS_DEATH_KNIGHT:
                            case CLASS_ROGUE:
                            case CLASS_HUNTER:
                                return true;
                            case CLASS_DRUID:
                                if (unit->ToPlayer()->GetSpecializationId() == SPEC_DRUID_BEAR)
                                    return true;
                                return false;
                            case CLASS_PALADIN:
                                if (unit->ToPlayer()->GetSpecializationId() == SPEC_PALADIN_HOLY)
                                    return false;
                                return true;
                            default:
                                return false;
                        }

                        return false;
                    }
            };
        };
       
        SpellScript* GetSpellScript() const
        {
            return new spell_asira_dawnslayer_mark_of_silence_SpellScript();
        }
};

void AddSC_boss_asira_dawnslayer()
{
    new boss_asira_dawnslayer();
    new spell_asira_dawnslayer_blade_barrier();
    new spell_asira_dawnslayer_throw_knife();
    new spell_asira_dawnslayer_mark_of_silence();
}