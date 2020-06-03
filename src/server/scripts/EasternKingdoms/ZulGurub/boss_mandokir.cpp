#include "zulgurub.h"

enum ScriptTexts
{
    SAY_AGGRO   = 0,
    SAY_DEATH   = 1,
    SAY_KILL    = 2,
    SAY_LEVEL   = 3,
    SAY_OHGAN   = 4,
    SAY_RES     = 5,
};

enum Spells
{
    SPELL_DECAPITATE            = 96684,
    SPELL_LEVEL_UP              = 96662,
    SPELL_DEVASTATING_SLAM      = 96740,
    SPELL_BLOODLETTING_DUMMY    = 96776,
    SPELL_BLOODLETTING_DMG      = 96777,
    SPELL_BLOODLETTING_HEAL     = 96778,
    SPELL_REANIMATE_OHGAN       = 96724,
    SPELL_FRENZY                = 96800,
    SPELL_OHGAN_ORGERS          = 96721,
    SPELL_OHGAN_ORDERS_DMG      = 96722,

    SPELL_OHGAN_HEART           = 96727,

    SPELL_SPIRIT_FORM1          = 96568,
    SPELL_SPIRIT_FORM2          = 96642,
    SPELL_RESSURECT             = 96484,

};

enum Events
{
    EVENT_DECAPITATE        = 1,
    EVENT_DISMOUNT          = 2,
    EVENT_BLOODLETTING      = 3,
    EVENT_RESSURECT         = 4,
    EVENT_OHGAN_ORDERS      = 5,
    EVENT_DEVASTATING_SLAM  = 6,
    EVENT_OHGAN_RES         = 7,
    EVENT_LEVEL_UP          = 8,
};

enum Adds
{
    NPC_OHGAN               = 52157,
    NPC_DEVASTATING_SLAM    = 52324,
    NPC_CHAINED_SPIRIT      = 52156,
};

enum Points
{
    POINT_RES,
};
enum Other
{
    DATA_RES    = 1,
};

const Position chainedspiritPos[8] = 
{
    {-12378.6f, -1861.22f, 127.542f, 5.34071f},
    {-12391.2f, -1905.27f, 127.32f, 0.610865f},
    {-12351.9f, -1861.51f, 127.481f, 4.67748f},
    {-12326.7f, -1904.33f, 127.411f, 2.75762f},
    {-12347.4f, -1917.54f, 127.32f, 1.55334f},
    {-12397.8f, -1887.73f, 127.545f, 0.0349066f},
    {-12372.4f, -1918.84f, 127.343f, 1.15192f},
    {-12330.3f, -1878.41f, 127.32f, 3.89208f}
};

class boss_mandokir : public CreatureScript
{
    public:

        boss_mandokir() : CreatureScript("boss_mandokir") { }
        
        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new boss_mandokirAI(pCreature);
        }
            
        struct boss_mandokirAI : public BossAI
        {
            boss_mandokirAI(Creature* pCreature) : BossAI(pCreature, DATA_MANDOKIR) 
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

                for (uint8 i = 0; i < 8; ++i)
                    me->SummonCreature(NPC_CHAINED_SPIRIT, chainedspiritPos[i]);
            }

            void KilledUnit(Unit* victim)
            {
                Talk(SAY_KILL);
                if (Creature* spirit = me->FindNearestCreature(NPC_CHAINED_SPIRIT, 200.0f))
                {
                    if(spirit->AI() && victim)
                        spirit->AI()->SetGUID(victim->GetGUID(), DATA_RES);
                }
            }

            void EnterCombat(Unit* /*who*/)
            {
                Talk(SAY_AGGRO);
                events.RescheduleEvent(EVENT_DECAPITATE, 7000);
                events.RescheduleEvent(EVENT_BLOODLETTING, 10000);
                events.RescheduleEvent(EVENT_DISMOUNT, 15000);
                DoZoneInCombat();
                instance->SetBossState(DATA_MANDOKIR, IN_PROGRESS);
            }

            void JustDied(Unit* /*killer*/)
            {
                _JustDied();
                Talk(SAY_DEATH);
            }
            
            void SummonedCreatureDies(Creature* summon, Unit* killer)
            {
                if (summon->GetEntry() == NPC_OHGAN)
                    events.RescheduleEvent(EVENT_OHGAN_RES, 10000);
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
                        case EVENT_DECAPITATE:
                        {
                            Unit* pTarget = NULL;
                            pTarget = SelectTarget(SELECT_TARGET_RANDOM, 1, 0.0f, true);
                            /*if (!pTarget)
                                pTarget = pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true);*/
                            if (pTarget)
                            {
                                DoCast(pTarget, SPELL_DECAPITATE);
                                events.RescheduleEvent(EVENT_LEVEL_UP, 5000);
                            }
                            events.RescheduleEvent(EVENT_DECAPITATE, 45000);
                            break;
                        }
                        case EVENT_LEVEL_UP:
                            Talk(SAY_LEVEL);
                            DoCast(me, SPELL_LEVEL_UP);
                            break;
                        case EVENT_BLOODLETTING:
                        {
                            Unit* pTarget = NULL;
                            pTarget = SelectTarget(SELECT_TARGET_RANDOM, 1, 0.0f, true);
                            if (!pTarget)
                                pTarget = pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true);
                            if (pTarget)
                            {
                                DoCast(pTarget, SPELL_BLOODLETTING_DUMMY);
                                me->ClearUnitState(UNIT_STATE_CASTING);
                            }
                            events.RescheduleEvent(EVENT_BLOODLETTING, urand(20000, 30000));
                            break;
                        }
                        case EVENT_OHGAN_RES:
                            Talk(SAY_RES);
                            break;
                        case EVENT_OHGAN_ORDERS:
                            break;
                        case EVENT_DISMOUNT:
                            Talk(SAY_OHGAN);
                            me->Dismount();
                            me->SummonCreature(NPC_OHGAN, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation());
                            break;
                    }
                }
            

                DoMeleeAttackIfReady();
            }
        };
};

class npc_mandokir_chained_spirit : public CreatureScript
{
    public:

        npc_mandokir_chained_spirit() : CreatureScript("npc_mandokir_chained_spirit") {}
        
        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_mandokir_chained_spiritAI(pCreature);
        }

        struct npc_mandokir_chained_spiritAI : public ScriptedAI
        {
            npc_mandokir_chained_spiritAI(Creature* pCreature) : ScriptedAI(pCreature) 
            {
                me->SetSpeed(MOVE_RUN, 0.9f);
                me->SetReactState(REACT_PASSIVE);
                DoCast(me, SPELL_SPIRIT_FORM1, true);
                DoCast(me, SPELL_SPIRIT_FORM2, true);
            }
            
            InstanceScript* instance;
            ObjectGuid playerGUID;
            uint32 despawnTimer;
            bool res;

            void Reset()
            {
                playerGUID.Clear();
                res = false;
                despawnTimer = 5000;
            }

            void SetGUID(ObjectGuid const& guid, int32 data)
            {
                if (playerGUID)
                    return;
                playerGUID = guid;
                if (Unit* player = ObjectAccessor::GetUnit(*me, playerGUID))
                    me->GetMotionMaster()->MovePoint(POINT_RES, player->GetPositionX(), player->GetPositionY(), player->GetPositionZ());
            }

            void MovementInform(uint32 type, uint32 id)
            {
                if (id == POINT_RES)
                {
                    if (Unit* player = ObjectAccessor::GetUnit(*me, playerGUID))
                    {
                        res = true;
                        despawnTimer = 5000;
                        DoCast(player, SPELL_RESSURECT);
                    }
                }
            }

            void UpdateAI(uint32 diff)
            {
                if (res)
                {
                    if (despawnTimer <= diff)
                    {
                        res = false;
                        me->DespawnOrUnsummon();
                    }
                    else
                        despawnTimer -= diff;
                }
            }

        };
};

class npc_mandokir_ohgan : public CreatureScript
{
    public:

        npc_mandokir_ohgan() : CreatureScript("npc_mandokir_ohgan") {}
        
        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_mandokir_ohganAI(pCreature);
        }

        struct npc_mandokir_ohganAI : public ScriptedAI
        {
            npc_mandokir_ohganAI(Creature* pCreature) : ScriptedAI(pCreature) 
            {
                me->SetSpeed(MOVE_RUN, 0.8f);
                me->SetReactState(REACT_PASSIVE);
                DoCast(me, SPELL_OHGAN_HEART);
            }
            
            void Reset()
            {
            }

            void SetGUID(ObjectGuid const& guid, int32 data)
            {
            }

            void MovementInform(uint32 type, uint32 id)
            {
            }

            void UpdateAI(uint32 diff)
            {
            }

        };
};

class spell_mandokir_bloodletting : public SpellScriptLoader
{
    public:
        spell_mandokir_bloodletting() : SpellScriptLoader("spell_mandokir_bloodletting") { }

        class spell_mandokir_bloodletting_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_mandokir_bloodletting_AuraScript);

            void PeriodicTick(AuraEffect const* aurEff)
            {
                if (!GetCaster() || !GetTarget())
                    return;

                float damage = 0.5f * GetTarget()->GetHealth();
                float heal = 0.5f * damage;
                GetCaster()->CastCustomSpell(GetTarget(), SPELL_BLOODLETTING_DMG, &damage, 0, 0, true);
                GetCaster()->CastCustomSpell(GetCaster(), SPELL_BLOODLETTING_HEAL, &heal, 0, 0, true);
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_mandokir_bloodletting_AuraScript::PeriodicTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_mandokir_bloodletting_AuraScript();
        }
};

void AddSC_boss_mandokir()
{
    new boss_mandokir();
    new npc_mandokir_chained_spirit();
    new npc_mandokir_ohgan();
    new spell_mandokir_bloodletting();
}
