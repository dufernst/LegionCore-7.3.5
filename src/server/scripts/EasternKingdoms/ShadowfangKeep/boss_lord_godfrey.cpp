#include"Spell.h"
#include"shadowfang_keep.h"

enum ScriptTexts
{
    SAY_AGGRO_ALLIANCE = 0,
    SAY_AGGRO_HORDE    = 1,
    SAY_DEATH          = 2,
    SAY_KILL           = 3,
};

enum Spells
{
    SPELL_PISTOL_BARRAGE                    = 93520,
    SPELL_PISTOL_BARRAGE_DUMMY              = 96345,
    SPELL_PISTOL_BARRAGE_AURA               = 93566,
    SPELL_PISTOL_BARRAGE_DMG                = 93564,
    SPELL_PISTOL_BARRAGE_MISSILE_SUM        = 96344,
    SPELL_PISTOL_BARRAGE_SUM                = 96343,
    SPELL_PISTOL_BARRAGE_DUMMY_1            = 93557,
    SPELL_PISTOL_BARRAGE_DUMMY_2            = 93558,
    SPELL_PISTOLS_VISUAL_PASSIVE            = 93597,
    SPELL_SUMMON_BLOODTHIRSTY_GHOULS        = 93707,
    SPELL_SUMMON_BLOODTHIRSTY_GHOULS_M      = 93709,
    SPELL_CURSED_BULLET                     = 93629,
    SPELL_MORTAL_WOUND                      = 93675,
    SPELL_ACHIEVEMENT_CREDIT                = 95929,
    SPELL_ACHIEVEMENT_RESET                 = 95930,
};

enum Events
{
    EVENT_PISTOL_BARRAGE                = 1,
    EVENT_SUMMON_BLOODTHIRSTY_GHOULS    = 2,
    EVENT_CURSED_BULLET                 = 3,
    EVENT_MORTAL_WOUND                  = 4,
};

enum Adds
{
    NPC_BLOODTHRISTY_GHOUL     = 50561,
    NPC_PISTOL_BARRAGE         = 52065,
};

class boss_lord_godfrey : public CreatureScript
{
    public:
        boss_lord_godfrey() : CreatureScript("boss_lord_godfrey") { }
        
        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new boss_lord_godfreyAI(pCreature);
        }

        struct boss_lord_godfreyAI : public BossAI
        {
            boss_lord_godfreyAI(Creature* pCreature) : BossAI(pCreature, DATA_GODFREY)
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
    
            void Reset()
            {
                _Reset();
                DoCast(SPELL_PISTOLS_VISUAL_PASSIVE);
            }
                
            void EnterCombat(Unit* pWho)
            {
                events.RescheduleEvent(EVENT_MORTAL_WOUND, 10000);
                events.RescheduleEvent(EVENT_CURSED_BULLET, 15000);
                events.RescheduleEvent(EVENT_SUMMON_BLOODTHIRSTY_GHOULS, 17000);
                events.RescheduleEvent(EVENT_PISTOL_BARRAGE, 22000);
                instance->SetBossState(DATA_GODFREY, IN_PROGRESS);
                if (instance->GetData(DATA_TEAM) == ALLIANCE)
                    Talk(SAY_AGGRO_ALLIANCE);
                else
                    Talk(SAY_AGGRO_HORDE);
                DoZoneInCombat();
            }
                
            void SpellHit(Unit* caster, SpellInfo const* spell)
            {
                if (spell->HasEffect(SPELL_EFFECT_INTERRUPT_CAST))
                    if (me->GetCurrentSpell(CURRENT_GENERIC_SPELL))
                        if (me->GetCurrentSpell(CURRENT_GENERIC_SPELL)->m_spellInfo->Id == SPELL_CURSED_BULLET)
                            me->InterruptSpell(CURRENT_GENERIC_SPELL, false);
            }

            void JustDied(Unit* who)
            {
                _JustDied();
                Talk(SAY_DEATH);
            }
    
            void UpdateAI(uint32 uiDiff)
            {
                if (!UpdateVictim())
                    return;
    
                events.Update(uiDiff);
    
                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;
                
                while(uint32 eventId = events.ExecuteEvent())
                {
                    switch(eventId)
                    {
                        case EVENT_MORTAL_WOUND:
                            DoCast(me->getVictim(), SPELL_MORTAL_WOUND);
                            events.RescheduleEvent(EVENT_MORTAL_WOUND, 10000);
                            break;
                        case EVENT_CURSED_BULLET:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1))
                                DoCast(target, SPELL_CURSED_BULLET);
                            events.RescheduleEvent(EVENT_CURSED_BULLET, 15000);
                            break;
                        case EVENT_SUMMON_BLOODTHIRSTY_GHOULS:
                            DoCast(SPELL_SUMMON_BLOODTHIRSTY_GHOULS);
                            events.RescheduleEvent(EVENT_SUMMON_BLOODTHIRSTY_GHOULS, 17000);
                            break;
                        case EVENT_PISTOL_BARRAGE:
                            me->CastCustomSpell(SPELL_PISTOL_BARRAGE_MISSILE_SUM, SPELLVALUE_MAX_TARGETS, 1);
                            DoCast(me, SPELL_PISTOL_BARRAGE, true);
                            me->SetReactState(REACT_PASSIVE);
                            me->AttackStop();
                            if (Creature* pistolTarget = me->FindNearestCreature(NPC_PISTOL_BARRAGE, 60.0f))
                                me->SetFacingToObject(pistolTarget);
                            events.RescheduleEvent(EVENT_PISTOL_BARRAGE, 20000);
                            break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };
};

class npc_godfrey_pistol_barrage : public CreatureScript
{
public:
    npc_godfrey_pistol_barrage() : CreatureScript("npc_godfrey_pistol_barrage") { }
    
    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_godfrey_pistol_barrageAI(pCreature);
    }
    struct npc_godfrey_pistol_barrageAI : public Scripted_NoMovementAI
    {
        npc_godfrey_pistol_barrageAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature)
        {
        }

        void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType dmgType)
        {
            damage = 0;
        }
    };
};

class npc_bloodthirsty_ghoul : public CreatureScript
{
public:
    npc_bloodthirsty_ghoul() : CreatureScript("npc_bloodthirsty_ghoul") { }
    
    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_bloodthirsty_ghoulAI(pCreature);
    }
    struct npc_bloodthirsty_ghoulAI : public ScriptedAI
    {
        npc_bloodthirsty_ghoulAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
        }

        void JustDied(Unit* killer)
        {
            if (IsHeroic() && me->ToTempSummon() && me->ToTempSummon()->GetSummoner())
                   if (Creature* Godfrey = me->ToTempSummon()->GetSummoner()->ToCreature())
                       if (killer == Godfrey)
                           DoCast(SPELL_ACHIEVEMENT_CREDIT);
        }
    };
};

class spell_godfrey_summon_bloodthirsty_ghouls : public SpellScriptLoader
{
    public:
        spell_godfrey_summon_bloodthirsty_ghouls() : SpellScriptLoader("spell_godfrey_summon_bloodthirsty_ghouls") { }

        class spell_godfrey_summon_bloodthirsty_ghouls_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_godfrey_summon_bloodthirsty_ghouls_AuraScript);

            void HandleDummyTick(AuraEffect const* aurEff)
            {
                GetCaster()->CastSpell(GetCaster(), SPELL_SUMMON_BLOODTHIRSTY_GHOULS_M, true);
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_godfrey_summon_bloodthirsty_ghouls_AuraScript::HandleDummyTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_godfrey_summon_bloodthirsty_ghouls_AuraScript();
        }
};

class spell_pistol_barrage : public SpellScriptLoader
{
    public:
        spell_pistol_barrage() : SpellScriptLoader("spell_pistol_barrage") { }

        class spell_pistol_barrage_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pistol_barrage_AuraScript);

            void OnPeriodic(AuraEffect const* aurEff)
            {
                float angle = frand(0, M_PI / 6);
                Position pos1;
                Position pos2;
                GetCaster()->GetNearPosition(pos1, 45.0f, angle);
                GetCaster()->GetNearPosition(pos2, 45.0f, -angle);
                GetCaster()->CastSpell(pos1.GetPositionX(), pos1.GetPositionY(), pos1.GetPositionZ(), SPELL_PISTOL_BARRAGE_DUMMY_1, true);
                GetCaster()->CastSpell(pos2.GetPositionX(), pos2.GetPositionY(), pos2.GetPositionZ(), SPELL_PISTOL_BARRAGE_DUMMY_2, true);
            }

            void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (GetCaster() && GetCaster()->ToCreature())
                    GetCaster()->ToCreature()->SetReactState(REACT_AGGRESSIVE);
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_pistol_barrage_AuraScript::OnPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
                OnEffectRemove += AuraEffectRemoveFn(spell_pistol_barrage_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_pistol_barrage_AuraScript();
        }
};

void AddSC_boss_lord_godfrey()
{
    new boss_lord_godfrey();
    new npc_godfrey_pistol_barrage();
    new npc_bloodthirsty_ghoul();
    new spell_godfrey_summon_bloodthirsty_ghouls();
    new spell_pistol_barrage();
}