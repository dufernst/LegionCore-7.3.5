#include "baradin_hold.h"

enum Spells
{
    SPELL_BERSERK                   = 47008, 
    SPELL_SEARING_SHADOW            = 96913, 

    SPELL_FOCUSED_FIRE              = 96872,
    SPELL_FOCUSED_FIRE_DUMMY        = 96886,
    SPELL_FOCUSED_FIRE_1            = 96884,

    SPELL_EYE_OF_OCCUTHAR           = 96920, 
    SPELL_EYE_OF_OCCUTHAR_25        = 101006,
    SPELL_EYE_OF_OCCUTHAR_VEHICLE   = 96932,

    SPELL_EYE_OF_OCCUTHAR_DMG       = 96942,

    SPELL_OCCUTHARS_DESTRUCTION     = 96968,
    
};

enum Events
{
    EVENT_BERSERK           = 1,
    EVENT_SEARING_SHADOWS   = 2,
    EVENT_EYE_OF_OCCUTHAR   = 3,
    EVENT_FOCUSED_FIRE      = 4,
};

enum Adds
{
    NPC_EYE_OF_OCCUTHAR = 52389,
    NPC_EYESTALK        = 52369,
};

class boss_occuthar : public CreatureScript
{
    public:
        boss_occuthar() : CreatureScript("boss_occuthar") { }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return GetInstanceAI<boss_occutharAI>(pCreature);
        }

        struct boss_occutharAI : public BossAI
        {
            boss_occutharAI(Creature* pCreature) : BossAI(pCreature, DATA_OCCUTHAR) 
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
            }

            void Reset()
            {
                _Reset();
                
                events.Reset();
            }

            void JustReachedHome()
            {
                _JustReachedHome();
            }

            void EnterCombat(Unit* attacker)
            {
                events.RescheduleEvent(EVENT_BERSERK, 300000);
                events.RescheduleEvent(EVENT_EYE_OF_OCCUTHAR, 59000);
                events.RescheduleEvent(EVENT_FOCUSED_FIRE, 26000);
                events.RescheduleEvent(EVENT_SEARING_SHADOWS, 24000); 
                instance->SetBossState(DATA_OCCUTHAR, IN_PROGRESS);
            }

            void JustDied(Unit* killer)
            {
                _JustDied();
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
                        case EVENT_BERSERK:
                            DoCast(me, SPELL_BERSERK, true);
                            break;
                        case EVENT_SEARING_SHADOWS:
                            DoCast(me->getVictim(), SPELL_SEARING_SHADOW);
                            events.RescheduleEvent(EVENT_SEARING_SHADOWS, 24000);
                            break;
                        case EVENT_FOCUSED_FIRE:
                            DoCastAOE(SPELL_FOCUSED_FIRE);
                            events.RescheduleEvent(EVENT_FOCUSED_FIRE, 14000);
                            break;
                        case EVENT_EYE_OF_OCCUTHAR:
                            DoCastAOE(SPELL_EYE_OF_OCCUTHAR);
                            events.RescheduleEvent(EVENT_FOCUSED_FIRE, 26000);
                            events.RescheduleEvent(EVENT_EYE_OF_OCCUTHAR, 59000);
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }
        };
};

class npc_occuthar_eyestalk : public CreatureScript
{
    public:
        npc_occuthar_eyestalk() : CreatureScript("npc_occuthar_eyestalk") { }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return GetInstanceAI<npc_occuthar_eyestalkAI>(pCreature);
        }

        struct npc_occuthar_eyestalkAI : public Scripted_NoMovementAI
        {
            npc_occuthar_eyestalkAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature)
            {
                //me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                //me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                me->SetReactState(REACT_PASSIVE);
            }

            void IsSummonedBy(Unit* owner)
            {
                if (Creature* pOccuthar = me->FindNearestCreature(NPC_OCCUTHAR, 200.0f))
                {
                    if (pOccuthar->isInCombat())
                        me->SetInCombatWithZone();

                    pOccuthar->CastSpell(me, SPELL_FOCUSED_FIRE_1, true);
                }
            }
        };
};

class npc_occuthar_eye_of_occuthar : public CreatureScript
{
    public:
        npc_occuthar_eye_of_occuthar() : CreatureScript("npc_occuthar_eye_of_occuthar") { }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return GetInstanceAI<npc_occuthar_eye_of_occutharAI>(pCreature);
        }

        struct npc_occuthar_eye_of_occutharAI : public Scripted_NoMovementAI
        {
            npc_occuthar_eye_of_occutharAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature)
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
                me->SetReactState(REACT_PASSIVE);
                uiDestructionTimer = 9500;
                bDestruction = false;
            }

            uint32 uiDestructionTimer;
            bool bDestruction;

            void UpdateAI(uint32 diff)
            {
                if (uiDestructionTimer <= diff && !bDestruction)
                {
                    bDestruction = true;
                    DoCastAOE(SPELL_OCCUTHARS_DESTRUCTION, true);
                }
                else
                    uiDestructionTimer -= diff;
            }

        };
};

class spell_occuthar_eye_of_occuthar : public SpellScriptLoader
{
    public:
        spell_occuthar_eye_of_occuthar() : SpellScriptLoader("spell_occuthar_eye_of_occuthar") { }

        class spell_occuthar_eye_of_occuthar_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_occuthar_eye_of_occuthar_SpellScript);

            void HandleScript(SpellEffIndex /*effIndex*/)
            {
                if (!GetCaster() || !GetHitUnit())
                    return;

                GetCaster()->CastSpell(GetCaster(), SPELL_EYE_OF_OCCUTHAR_VEHICLE, true);
                
                if (Creature* pEye = GetCaster()->SummonCreature(NPC_EYE_OF_OCCUTHAR, GetCaster()->GetPositionX(), GetCaster()->GetPositionY(), GetCaster()->GetPositionZ(), GetCaster()->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 11000))
                    pEye->CastSpell(GetCaster(), SPELL_EYE_OF_OCCUTHAR_DMG, true); 
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_occuthar_eye_of_occuthar_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_occuthar_eye_of_occuthar_SpellScript();
        }
};

void AddSC_boss_occuthar()
{
    new boss_occuthar();
    new npc_occuthar_eyestalk();
    new npc_occuthar_eye_of_occuthar();
    new spell_occuthar_eye_of_occuthar();
}
