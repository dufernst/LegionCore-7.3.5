#include "zulaman.h"

#define SE_LOC_X_MAX 400
#define SE_LOC_X_MIN 335
#define SE_LOC_Y_MAX 1435
#define SE_LOC_Y_MIN 1370

enum ScriptTexts
{
    SAY_AGGRO   = 0,
    SAY_SUMMON  = 1,
    SAY_KILL    = 2,
    SAY_DEATH   = 3,
};

enum Spells
{
    SPELL_STATIC_DISRUPTION = 44008,
    SPELL_CALL_LIGHTNING    = 43661,
    SPELL_ELECTRICAL_STORM  = 43648,
    SPELL_ELECTRICAL_SAFE   = 44007,
    SPELL_PLUCKED           = 97318,
    SPELL_EAGLE_SWOOP       = 44732,
};

enum Adds
{
    NPC_SOARING_EAGLE   = 24858,
    NPC_AMANI_KIDNAPPER = 52638,
};

enum Events
{
    EVENT_STATIC_DISRUPTION = 1,
    EVENT_CALL_LIGHTNING    = 2,
    EVENT_ELECTRICAL_STORM  = 3,
    EVENT_PLUCKED           = 4,
    EVENT_EAGLE_SWOOP       = 5,
    EVENT_GO_BACK           = 6,
    EVENT_SUMMON_EAGLE      = 7,
    EVENT_SUMMON_KIDNAPPER  = 8,
};

enum Points
{
    POINT_HOME  = 1,
};

class boss_akilzon : public CreatureScript
{
    public:
        boss_akilzon() : CreatureScript("boss_akilzon") {}
        
        CreatureAI* GetAI(Creature* pCreature) const
        {
            return GetInstanceAI<boss_akilzonAI>(pCreature);
        }

        struct boss_akilzonAI : public BossAI
        {
            boss_akilzonAI(Creature* pCreature) : BossAI(pCreature, DATA_AKILZON)
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
            
            ObjectGuid Eagles[8];

            void Reset()
            {
                _Reset();

                memset(&Eagles, 0, sizeof(Eagles));
            }

            void EnterCombat(Unit* /*who*/)
            {
                events.RescheduleEvent(EVENT_ELECTRICAL_STORM, 50000);
                events.RescheduleEvent(EVENT_STATIC_DISRUPTION, 7000);
                events.RescheduleEvent(EVENT_SUMMON_KIDNAPPER, 35000);
                events.RescheduleEvent(EVENT_CALL_LIGHTNING, urand(3000, 5000));
                events.RescheduleEvent(EVENT_SUMMON_EAGLE, 10000);
                Talk(SAY_AGGRO);
                DoZoneInCombat();
                instance->SetBossState(DATA_AKILZON, IN_PROGRESS);
            }

            void JustDied(Unit* /*killer*/)
            {
                _JustDied();
                Talk(SAY_DEATH);
            }
            
            void JustSummoned(Creature* summon)
            {
                BossAI::JustSummoned(summon);
            }
            
            void SummonedCreatureDespawn(Creature* summon)
            {
                BossAI::SummonedCreatureDespawn(summon);
            }

            void KilledUnit(Unit* /*victim*/)
            {
                Talk(SAY_KILL);
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
                        case EVENT_CALL_LIGHTNING:
                            DoCastVictim(SPELL_CALL_LIGHTNING);
                            events.RescheduleEvent(EVENT_CALL_LIGHTNING, urand(8000, 14000));
                            break;
                        case EVENT_STATIC_DISRUPTION:
                            if (Unit* pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                                DoCast(pTarget, SPELL_STATIC_DISRUPTION);
                            events.RescheduleEvent(EVENT_STATIC_DISRUPTION, urand(8000, 12000));
                            break;
                        case EVENT_ELECTRICAL_STORM:
                            if (Unit* pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                                DoCast(pTarget, SPELL_ELECTRICAL_STORM);
                            events.RescheduleEvent(EVENT_ELECTRICAL_STORM, 58000);
                            events.RescheduleEvent(EVENT_SUMMON_EAGLE, 13000);
                            events.RescheduleEvent(EVENT_STATIC_DISRUPTION, urand(13000, 17000));
                            events.RescheduleEvent(EVENT_CALL_LIGHTNING, urand(13000, 20000));
                            events.RescheduleEvent(EVENT_SUMMON_KIDNAPPER, urand(40000, 49000));
                            break;
                        case EVENT_SUMMON_EAGLE:
                            Talk(SAY_SUMMON);
                            for (uint8 i = 0; i < 8; i++)
                            {
                                if (!Unit::GetUnit(*me, Eagles[i]))
                                {
                                    if (Unit* pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                                        if (Creature* pEagle = me->SummonCreature(NPC_SOARING_EAGLE, pTarget->GetPositionX() + irand(-10, 10), pTarget->GetPositionY() + irand(-10, 10), pTarget->GetPositionZ() + urand(10, 15), pTarget->GetOrientation(), TEMPSUMMON_CORPSE_DESPAWN, 0))
                                            Eagles[i] = pEagle->GetGUID();
                                }
                            }
                            break;
                        case EVENT_SUMMON_KIDNAPPER:
                            if (Unit* pTarget = SelectTarget(SELECT_TARGET_RANDOM, 1, 0.0f, true, -SPELL_PLUCKED))
                                if (Creature* pKidnapper = me->SummonCreature(NPC_AMANI_KIDNAPPER, pTarget->GetPositionX(), pTarget->GetPositionY(), pTarget->GetPositionZ() + 3, pTarget->GetOrientation(), TEMPSUMMON_DEAD_DESPAWN, 5000))
                                    pTarget->CastSpell(pKidnapper, SPELL_PLUCKED, true);
                                
                            break;
                     }
                }

                DoMeleeAttackIfReady();
            }
        };
};

class npc_akilzon_soaring_eagle : public CreatureScript
{
    public:
        npc_akilzon_soaring_eagle() : CreatureScript("npc_akilzon_soaring_eagle") { }
        
        CreatureAI* GetAI(Creature* pCreature) const
        {
            return GetInstanceAI<npc_akilzon_soaring_eagleAI>(pCreature);
        }

        struct npc_akilzon_soaring_eagleAI : public ScriptedAI
        {
            npc_akilzon_soaring_eagleAI(Creature* pCreature) : ScriptedAI(pCreature) 
            {
                me->SetReactState(REACT_PASSIVE);
            }

            EventMap events;
            Position homePos;

            void Reset()
            {
                me->SetCanFly(true);
                me->GetPosition(&homePos);
                events.Reset();
            }
            
            void EnterCombat(Unit* who) 
            {
                events.RescheduleEvent(EVENT_EAGLE_SWOOP, urand(100, 6000));
            }

            void MovementInform(uint32 type, uint32 id)
            { 
                if (type == POINT_MOTION_TYPE)
                {
                    if (id == EVENT_CHARGE)
                    {
                        events.RescheduleEvent(EVENT_GO_BACK, 3000);
                    }
                    else if (id == POINT_HOME)
                    {
                        events.RescheduleEvent(EVENT_EAGLE_SWOOP, urand(2000, 10000));
                    }
                }
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
                        case EVENT_GO_BACK:
                            me->GetMotionMaster()->MovePoint(POINT_HOME, homePos);
                            break;
                        case EVENT_EAGLE_SWOOP:
                            if (Unit* pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                                DoCast(pTarget, SPELL_EAGLE_SWOOP);
                            break;
                     }
                }
            }
        };
};

class npc_akilzon_amani_kidnapper : public CreatureScript
{
    public:
        npc_akilzon_amani_kidnapper() : CreatureScript("npc_akilzon_amani_kidnapper") { }
        
        CreatureAI* GetAI(Creature* pCreature) const
        {
            return GetInstanceAI<npc_akilzon_amani_kidnapperAI>(pCreature);
        }

        struct npc_akilzon_amani_kidnapperAI : public ScriptedAI
        {
            npc_akilzon_amani_kidnapperAI(Creature* pCreature) : ScriptedAI(pCreature) 
            {
                me->SetReactState(REACT_PASSIVE);
            }

            void Reset()
            {
                me->SetCanFly(true);
            }
        };
};

class spell_akilzon_electrical_storm : public SpellScriptLoader
{
    public:
        spell_akilzon_electrical_storm() :  SpellScriptLoader("spell_akilzon_electrical_storm") { }

        class spell_akilzon_electrical_storm_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_akilzon_electrical_storm_AuraScript);

            void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (!GetTarget())
                    return;
               
                if (Player* pPlayer = GetTarget()->ToPlayer())
                {
                    pPlayer->CastSpell(pPlayer, SPELL_ELECTRICAL_SAFE, true);
                    pPlayer->SetClientControl(pPlayer, 0);
                    pPlayer->SetCanFly(true);
                    pPlayer->AddUnitMovementFlag(MOVEMENTFLAG_DISABLE_GRAVITY);
                    pPlayer->MonsterMoveWithSpeed(pPlayer->GetPositionX(), pPlayer->GetPositionY(), pPlayer->GetPositionZ() + 12, 15000);
                }
            }

            void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (!GetTarget())
                    return;

                if (Player* pPlayer = GetTarget()->ToPlayer())
                {
                    pPlayer->SetClientControl(pPlayer, 1);
                    pPlayer->SetCanFly(false);
                    pPlayer->RemoveUnitMovementFlag(MOVEMENTFLAG_DISABLE_GRAVITY);
                }                
            }

            void Register()
            {
                OnEffectApply += AuraEffectApplyFn(spell_akilzon_electrical_storm_AuraScript::OnApply, EFFECT_1, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
                OnEffectRemove += AuraEffectRemoveFn(spell_akilzon_electrical_storm_AuraScript::OnRemove, EFFECT_1, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_akilzon_electrical_storm_AuraScript();
        }
};

class ElectricalSafeCheck
{
    public:
        explicit ElectricalSafeCheck() {}

        bool operator()(WorldObject* object) const
        {
            return object->ToUnit()->HasAura(SPELL_ELECTRICAL_SAFE);
        }
};

class spell_akilzon_electrical_storm_dmg : public SpellScriptLoader
{
    public:
        spell_akilzon_electrical_storm_dmg() : SpellScriptLoader("spell_akilzon_electrical_storm_dmg") { }

        class spell_akilzon_electrical_storm_dmg_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_akilzon_electrical_storm_dmg_SpellScript);

            void CheckTarget(std::list<WorldObject*>& unitList)
            {
                unitList.remove_if(ElectricalSafeCheck());
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_akilzon_electrical_storm_dmg_SpellScript::CheckTarget, EFFECT_0, TARGET_UNIT_SRC_AREA_ALLY);
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_akilzon_electrical_storm_dmg_SpellScript::CheckTarget, EFFECT_1, TARGET_UNIT_SRC_AREA_ALLY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_akilzon_electrical_storm_dmg_SpellScript();
        }
};


void AddSC_boss_akilzon()
{
    new boss_akilzon();
    new npc_akilzon_soaring_eagle();
    //new npc_akilzon_amani_kidnapper();
    new spell_akilzon_electrical_storm();
    new spell_akilzon_electrical_storm_dmg();
}

