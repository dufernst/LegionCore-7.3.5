#include"shadowfang_keep.h"

enum ScriptTexts
{
    SAY_AGGRO = 0,
    SAY_DEATH = 1,
    SAY_KILL  = 2,
};

enum Events
{
    EVENT_VEIL_OF_SHADOW    = 1,
    EVENT_CURSED_VEIL_H     = 3,
};

enum Spells
{
    SPELL_VEIL_OF_SHADOW        = 23224,
    SPELL_CURSED_VEIL_H         = 93956,
    SPELL_SUMMON_WORGEN_SPIRIT  = 93857,

    SPELL_NANDOS_T              = 93899,
    SPELL_ODO_T                 = 93864,
    SPELL_RETHILGORE_T          = 93927,
    SPELL_RAZORCLAW_T           = 93924,
};

enum Adds
{
    NPC_RETHILGORE_DUMMY = 50085,
    NPC_RAZORCLAW_DUMMY  = 51080,
    NPC_ODO_DUMMY        = 50934,
    NPC_NANDOS_DUMMY     = 51047,
    NPC_ODO              = 50857,
    NPC_RAZORCLAW        = 50869,
    NPC_RETHILGORE       = 50834,
    NPC_NANDOS           = 50851,
};

class boss_baron_silverlaine : public CreatureScript
{
    public:
        boss_baron_silverlaine() : CreatureScript("boss_baron_silverlaine") { }
        
        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new boss_baron_silverlaineAI(pCreature);
        }
        struct boss_baron_silverlaineAI : public BossAI
        {
            boss_baron_silverlaineAI(Creature* pCreature) : BossAI(pCreature, DATA_SILVERLAINE)
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

            uint8 phase;

            void Reset()
            {
                _Reset();
                phase = 0;
            }

            void EnterCombat(Unit* pWho)
            {
                Talk(SAY_AGGRO);
                events.RescheduleEvent(EVENT_VEIL_OF_SHADOW, 12000);
                DoZoneInCombat();
                instance->SetBossState(DATA_SILVERLAINE, IN_PROGRESS);
            }

            void KilledUnit(Unit* who)
            {
                Talk(SAY_KILL);
            }

            void JustDied(Unit* pWho)
            {
                _JustDied();
                Talk(SAY_DEATH);
            }
                                 
            void UpdateAI(uint32 uiDiff)
            {
                if (!UpdateVictim())
                    return;

                if (!HealthAbovePct(50) && phase == 0)
                {
                    phase = 1;
                    DoCast(SPELL_SUMMON_WORGEN_SPIRIT);
                    return;
                }
                if (!HealthAbovePct(25) && phase == 1)
                {
                    phase = 2;
                    DoCast(SPELL_SUMMON_WORGEN_SPIRIT);
                    return;
                }

                events.Update(uiDiff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;
                    
                while(uint32 eventId = events.ExecuteEvent())
                {
                    switch(eventId)
                    {
                        case EVENT_VEIL_OF_SHADOW:
                            DoCast(me, SPELL_VEIL_OF_SHADOW);
                            events.RescheduleEvent(EVENT_VEIL_OF_SHADOW, 12000);
                            break;
                    }
                }
                DoMeleeAttackIfReady();
            }
         };
};

class npc_silverlaine_worgen : public CreatureScript
{
    public:
        npc_silverlaine_worgen() : CreatureScript("npc_silverlaine_worgen") { }
        
        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_silverlaine_worgenAI(pCreature);
        }
         struct npc_silverlaine_worgenAI : public ScriptedAI
         {
            npc_silverlaine_worgenAI(Creature* pCreature) : ScriptedAI(pCreature)
            {
                instance = pCreature->GetInstanceScript();
            }

            InstanceScript *instance;

            void IsSummonedBy(Unit* summoner)
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
                    AttackStart(target);
            }

            void UpdateAI(uint32 diff)
            {
                if (instance && instance->GetBossState(DATA_SILVERLAINE) != IN_PROGRESS)
                    me->DespawnOrUnsummon();

                DoMeleeAttackIfReady();
            }
         };
};

class npc_silverlaine_worgen_spirit : public CreatureScript
{
    public:
        npc_silverlaine_worgen_spirit() : CreatureScript("npc_silverlaine_worgen_spirit") { }
        
        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_silverlaine_worgen_spiritAI(pCreature);
        }
         struct npc_silverlaine_worgen_spiritAI : public Scripted_NoMovementAI
         {
            npc_silverlaine_worgen_spiritAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature)
            {
            }

            void IsSummonedBy(Unit* summoner)
            {
                switch (me->GetEntry())
                {
                    case NPC_ODO_DUMMY:
                        DoCast(me, SPELL_ODO_T);
                        break;
                    case NPC_RETHILGORE_DUMMY:
                        DoCast(me, SPELL_RETHILGORE_T);
                        break;
                    case NPC_NANDOS_DUMMY:
                        DoCast(me, SPELL_NANDOS_T);
                        break;
                    case NPC_RAZORCLAW_DUMMY:
                        DoCast(me, SPELL_RAZORCLAW_T);
                        break;    
                }
            }
         };
};

class spell_silverlaine_summon_worgen_spirit : public SpellScriptLoader
{
    public:
        spell_silverlaine_summon_worgen_spirit() : SpellScriptLoader("spell_silverlaine_summon_worgen_spirit") { }


        class spell_silverlaine_summon_worgen_spirit_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_silverlaine_summon_worgen_spirit_SpellScript);


            void HandleScript(SpellEffIndex effIndex)
            {
                if (!GetCaster())
                    return;

                switch (urand(0, 3))
                {
                case 0:
                    GetCaster()->CastSpell(GetCaster(), 93925, true);
                    break;
                case 1:
                    GetCaster()->CastSpell(GetCaster(), 93921, true);
                    break;
                case 2:
                    GetCaster()->CastSpell(GetCaster(), 93859, true);
                    break;
                case 3:
                    GetCaster()->CastSpell(GetCaster(), 93896, true);
                    break;
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_silverlaine_summon_worgen_spirit_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_silverlaine_summon_worgen_spirit_SpellScript();
        }
};


void AddSC_boss_baron_silverlaine()
{
    new boss_baron_silverlaine();
    new npc_silverlaine_worgen();
    new npc_silverlaine_worgen_spirit();
    new spell_silverlaine_summon_worgen_spirit();
}
