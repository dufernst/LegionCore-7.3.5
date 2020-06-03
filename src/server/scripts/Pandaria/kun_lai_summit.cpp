#include "ScriptedEscortAI.h"
#include "SpellAuraEffects.h"
#include "SpellScript.h"

enum eNessosSpells
{
    SPELL_VICIOUS_REND        = 125624,
    SPELL_GRAPPLING_HOOK      = 125623,
    SPELL_VANISH              = 125632,
    SPELL_SMOKED_BLADE        = 125633,
};

enum eNessosEvents
{
    EVENT_VICIOUS_REND        = 1,
    EVENT_GRAPPLING_HOOK      = 2,
    EVENT_VANISH              = 3,
    EVENT_SMOKED_BLADE        = 4,
};

class mob_nessos_the_oracle : public CreatureScript
{
    public:
        mob_nessos_the_oracle() : CreatureScript("mob_nessos_the_oracle") 
        { 
        }

        CreatureAI* GetAI(Creature* creature) const
        {
            return new mob_nessos_the_oracleAI(creature);
        }

        struct mob_nessos_the_oracleAI : public ScriptedAI
        {
            mob_nessos_the_oracleAI(Creature* creature) : ScriptedAI(creature)
            {
            }

            EventMap events;

            void Reset()
            {
                events.Reset();
                
                events.RescheduleEvent(EVENT_VICIOUS_REND,      7000);
                events.RescheduleEvent(EVENT_GRAPPLING_HOOK, 17000);
                events.RescheduleEvent(EVENT_VANISH, 12000);
            }

            void JustDied(Unit* /*killer*/)
            {
            }

            void UpdateAI(uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);
                

                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_VICIOUS_REND:
                            if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                                me->CastSpell(target, SPELL_VICIOUS_REND, false);
                            events.RescheduleEvent(EVENT_VICIOUS_REND,      7000);
                            break;
                        case EVENT_GRAPPLING_HOOK:
                            if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                                me->CastSpell(target, SPELL_GRAPPLING_HOOK, false);
                            events.RescheduleEvent(EVENT_GRAPPLING_HOOK, 17000);
                            break;
                        case EVENT_VANISH:
                            me->CastSpell(me, SPELL_VANISH, false);
                            events.RescheduleEvent(EVENT_VANISH, 20000);
                            events.RescheduleEvent(EVENT_SMOKED_BLADE, urand(0, 8000));
                            break;
                        case EVENT_SMOKED_BLADE:
                            if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                                me->CastSpell(target, SPELL_SMOKED_BLADE, false);

                            break;
                        default:
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }
        };
};

enum eSkiThikSpells
{
    SPELL_BLADE_FURY    = 125370,
    SPELL_TORNADO       = 125398,
    SPELL_TORNADO_DMG   = 131693,
    SPELL_WINDSONG      = 125373,
};

enum eSkiThikEvents
{
    EVENT_BLADE_FURY    = 1,
    EVENT_TORNADO       = 2,
    EVENT_WINDSONG      = 3,
};

class mob_ski_thik : public CreatureScript
{
    public:
        mob_ski_thik() : CreatureScript("mob_ski_thik") 
        { 
        }

        CreatureAI* GetAI(Creature* creature) const
        {
            return new mob_ski_thikAI(creature);
        }

        struct mob_ski_thikAI : public ScriptedAI
        {
            mob_ski_thikAI(Creature* creature) : ScriptedAI(creature)
            {
            }

            EventMap events;

            void Reset()
            {
                events.Reset();
                
                events.RescheduleEvent(EVENT_BLADE_FURY,       8000);
                events.RescheduleEvent(EVENT_TORNADO,         40000);
                events.RescheduleEvent(EVENT_WINDSONG,        32000);
            }

            void JustDied(Unit* /*killer*/)
            {
            }

            void JustSummoned(Creature* summon)
            {
                if (summon->GetEntry() == 64267)
                {
                    summon->AddAura(SPELL_TORNADO_DMG, summon);
                    summon->SetReactState(REACT_PASSIVE);
                    summon->GetMotionMaster()->MoveRandom(20.0f);
                }
            }

            void UpdateAI(uint32 diff)
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
                        case EVENT_BLADE_FURY:
                            if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                                me->CastSpell(target, SPELL_BLADE_FURY, false);
                            events.RescheduleEvent(EVENT_BLADE_FURY,      8000);
                            break;
                        case EVENT_TORNADO:
                            if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                                me->CastSpell(target, SPELL_TORNADO, false);
                            events.RescheduleEvent(EVENT_TORNADO, 40000);
                            break;
                        case EVENT_WINDSONG:
                            if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                                me->CastSpell(target, SPELL_WINDSONG, false);
                            events.RescheduleEvent(EVENT_WINDSONG, 32000);
                            break;

                        default:
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }
        };
};

enum eMogujiaSoulCallerSpells
{
    SPELL_DRAIN_LIFE        =  84533,
    SPELL_SHADOW_BOLT       =   9613,
    SPELL_SHADOW_CRASH      = 129132,
};

enum eMogujiaSoulCallerEvents
{
    EVENT_DRAIN_LIFE        = 1,
    EVENT_SHADOW_BOLT       = 2,
    EVENT_SHADOW_CRASH      = 3,
};

class mob_mogujia_soul_caller : public CreatureScript
{
    public:
        mob_mogujia_soul_caller() : CreatureScript("mob_mogujia_soul_caller") 
        { 
        }

        CreatureAI* GetAI(Creature* creature) const
        {
            return new mob_mogujia_soul_callerAI(creature);
        }

        struct mob_mogujia_soul_callerAI : public ScriptedAI
        {
            mob_mogujia_soul_callerAI(Creature* creature) : ScriptedAI(creature)
            {
            }

            EventMap events;

            void Reset()
            {
                events.Reset();
                
                events.RescheduleEvent(EVENT_DRAIN_LIFE,      20000);
                events.RescheduleEvent(EVENT_SHADOW_BOLT,     15000);
                events.RescheduleEvent(EVENT_SHADOW_CRASH,    32000);
            }

            void UpdateAI(uint32 diff)
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
                        case EVENT_DRAIN_LIFE:
                            if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                                me->CastSpell(target, SPELL_DRAIN_LIFE, false);
                            events.RescheduleEvent(EVENT_DRAIN_LIFE,      20000);
                            break;
                        case EVENT_SHADOW_BOLT:
                            if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                                me->CastSpell(target, SPELL_SHADOW_BOLT, false);
                            events.RescheduleEvent(EVENT_SHADOW_BOLT, 15000);
                            break;
                        case EVENT_SHADOW_CRASH:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
                                me->CastSpell(target, SPELL_SHADOW_CRASH, false);
                            events.RescheduleEvent(EVENT_SHADOW_CRASH, 32000);
                            break;
                            
                        default:
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }
        };
};

// 66138
struct npc_master_cheng_q31840 : public ScriptedAI
{
    npc_master_cheng_q31840(Creature* creature) : ScriptedAI(creature) {}

    bool challengeDone = false;
    uint32 timer;

    void IsSummonedBy(Unit* summoner) override
    {
        me->SetLevel(summoner->getLevel());
        me->AddDelayedEvent(2000, [this]() -> void
        {
            Talk(1);
            me->SetWalk(true);
            me->GetMotionMaster()->MovePoint(1, 3943.22f, 1836.45f, 904.33f);
        });
    }

    void MovementInform(uint32 type, uint32 id) override
    {
        if (type != POINT_MOTION_TYPE)
            return;

        if (id == 1)
        {
            if (auto owner = me->GetAnyOwner())
            {
                me->SetHomePosition(me->GetPosition());
                me->SetFacingToObject(owner);
                me->AddDelayedEvent(1000, [this]() -> void
                {
                    Talk(2);
                });
                me->AddDelayedEvent(8000, [this]() -> void
                {
                    me->HandleEmoteCommand(2);
                });
                me->AddDelayedEvent(10000, [this]() -> void
                {
                    me->setFaction(14);
                });
            }
        }
    }

    void DamageTaken(Unit* attacker, uint32& damage, DamageEffectType dmgType) override
    {
        if (damage >= me->GetHealth())
        {
            damage = 0;
            if (!challengeDone)
            {
                challengeDone = true;
                me->StopAttack();
                if (auto unit = me->GetAnyOwner())
                {
                    if (auto plr = unit->ToPlayer())
                    {
                        Talk(4);
                        me->setFaction(35);
                        me->HandleEmoteCommand(2);
                        plr->KilledMonsterCredit(me->GetEntry());
                        me->DespawnOrUnsummon(6000);
                    }
                }
            }
        }
    }

    void OnEnterCombat(Player* player, Unit* /*target*/)
    {
        timer = urand(3000, 6000);
    }

    void Reset() override
    {
        timer = 0;
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        if (timer <= diff)
        {
            if (me->GetEntry() == 66138)
            {
                Talk(3);
                DoCast(me, 130073, true);
            }
            if (me->GetEntry() == 65899)
            {
                Talk(3);
                DoCast(129700);
            }
            if (me->GetEntry() == 66073)
            {
                Talk(3);
                DoCast(129952);
            }
            if (me->GetEntry() == 65977)
            {
                Talk(3);
                DoCast(131743);
            }
            timer = urand(15000, 17000);
        }
        else
            timer -= diff;

        DoMeleeAttackIfReady();
    }
};

// 130283
class spell_monk_exp_buff : public SpellScriptLoader
{
public:
    spell_monk_exp_buff() : SpellScriptLoader("spell_monk_exp_buff") {}

    class spell_monk_exp_buff_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_monk_exp_buff_AuraScript);

        uint32 m_timer = 3000;

        void CalculateAmount(AuraEffect const* /*aurEff*/, float& amount, bool& canBeRecalculated)
        {
            if (auto owner = GetCaster())
            {
                if (owner->getLevel() > 85)
                    amount = 20;
                if (owner->getLevel() > 89)
                    amount = 0;
            }
        }

        void CalculateAmount1(AuraEffect const* /*aurEff*/, float& amount, bool& canBeRecalculated)
        {
            if (auto owner = GetCaster())
            {
                if (owner->getLevel() < 90)
                    amount = 0;
            }
        }

        void OnUpdate(uint32 diff, AuraEffect* aurEff)
        {
            if (m_timer)
            {
                if (m_timer <= diff)
                {
                    m_timer = 3000;
                    if (auto caster = GetCaster())
                    {
                        if (auto plr = caster->ToPlayer())
                        {
                            if (plr->GetMap() && plr->GetMap()->IsBattlegroundOrArena())
                                caster->RemoveAurasDueToSpell(130283);
                        }
                    }
                }
                else
                    m_timer -= diff;
            }
        }

        void Register() override
        {
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_monk_exp_buff_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_MOD_XP_QUEST_PCT);
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_monk_exp_buff_AuraScript::CalculateAmount, EFFECT_1, SPELL_AURA_MOD_XP_PCT);
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_monk_exp_buff_AuraScript::CalculateAmount1, EFFECT_3, SPELL_AURA_MOD_RATING);
            OnEffectUpdate += AuraEffectUpdateFn(spell_monk_exp_buff_AuraScript::OnUpdate, EFFECT_0, SPELL_AURA_MOD_XP_QUEST_PCT);
            OnEffectUpdate += AuraEffectUpdateFn(spell_monk_exp_buff_AuraScript::OnUpdate, EFFECT_1, SPELL_AURA_MOD_XP_PCT);
            OnEffectUpdate += AuraEffectUpdateFn(spell_monk_exp_buff_AuraScript::OnUpdate, EFFECT_3, SPELL_AURA_MOD_RATING);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_monk_exp_buff_AuraScript();
    }
};


void AddSC_kun_lai_summit()
{
    new mob_nessos_the_oracle();
    new mob_ski_thik();
    new mob_mogujia_soul_caller();
    new spell_monk_exp_buff();
    RegisterCreatureAI(npc_master_cheng_q31840);
}
