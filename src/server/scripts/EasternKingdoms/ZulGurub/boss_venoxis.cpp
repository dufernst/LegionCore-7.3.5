//#include "GridNotifiers.h"
#include "zulgurub.h"
#include "ObjectVisitors.hpp"

enum ScriptTexts
{
    SAY_DEATH       = 0,
    SAY_TRANSFORM   = 1,
    SAY_BLOODVENOM  = 3, // ?
    SAY_X           = 2, // ?
    SAY_KILL        = 4,
    SAY_AGGRO       = 5,
};

enum Spells
{
    SPELL_TOXIC_LINK                = 96475,
    SPELL_TOXIC_LINK_AURA           = 96477, // On players
    SPELL_TOXIC_LINK_DUMMY          = 96476, // Beam on ally
    SPELL_TOXIC_EXPLOSION           = 96489,
    SPELL_WORD_OF_HETHISS           = 96560,
    SPELL_WHISPER_OF_HETHISS        = 96466,
    SPELL_WHISPER_OF_HETHISS_DMG    = 96469,
    SPELL_BREATH_OF_HETHISS         = 96509,
    SPELL_POOL_OF_ACID_TEARS        = 96515,
    SPELL_POOL_OF_ACID_TEARS_AURA   = 96520, // make scale
    SPELL_BLESSING_OF_THE_SNAKE_GOD = 96512,
    SPELL_BLOODVENOM                = 96842,
    SPELL_BLOODVENOM_SUMMON         = 96637,
    SPELL_BLOODVENOM_DUMMY          = 97110,
    SPELL_BLOODVENOM_AURA           = 97099,
    SPELL_VENOM_WITHDRAWAL          = 96653,
    SPELL_VENOMOUS_EFFUSION         = 96678, 
    SPELL_VENOMOUS_EFFUSION_SUMMON  = 96680,
    SPELL_VENOMOUS_EFFUSION_AURA    = 96681,
    SPELL_VENOMOUS_EFFUSION_DUMMY   = 96534,
    SPELL_VENOM_TOTEM_BEAM_RIGHT    = 96937,
    SPELL_VENOM_TOTEM_BEAM_LEFT     = 96936,
    SPELL_VENOM_TOTEM_BEAM_CENTER   = 97098,
};

enum Events
{
    EVENT_TOXIC_LINK            = 1,
    EVENT_WORD_OF_HETISS        = 2,
    EVENT_BREATH_OF_HETHISS     = 3,
    EVENT_POOL_OF_ACID_TEARS    = 4,
    EVENT_MOVE_DOWN             = 5,
    EVENT_VENOM_WITHDRAWAL      = 6,
    EVENT_TRANSFORM             = 7,
    EVENT_WHISPER_OF_HETHISS    = 8,
    EVENT_MOVE_UP               = 9,
    EVENT_BLOODVENOM            = 10,
};

enum Adds
{
    NPC_VENOMOUS_EFFUSION           = 52288, // 96681
    NPC_VENOMOUS_EFFUSION_STALKER   = 52302, // 96678
    NPC_POOL_OF_ACID_TEARS          = 52197, // 96520
    NPC_BLOODVENOM                  = 52525, // 97110
};

enum Points
{
    POINT_DOWN          = 1,
    POINT_UP            = 2,
    POINT_WITHDRAWAL    = 3,
};

const Position downPos = {-12000.54f, -1685.88f, 32.28f, 0.68f};
const Position upPos = {-12020.12f, -1699.96f, 39.53f, 0.60f};

const Position mazeleftPos[18] = 
{
    {-12003.12f, -1700.04f, 32.29f, 0.0f},
    {-11999.17f, -1705.85f, 32.29f, 0.0f},
    {-12000.37f, -1714.15f, 32.34f, 0.0f},
    {-11992.49f, -1719.70f, 32.39f, 0.0f},
    {-11985.03f, -1714.68f, 32.50f, 0.0f},
    {-11981.08f, -1718.16f, 32.95f, 0.0f},
    {-11975.54f, -1714.41f, 32.90f, 0.0f},
    {-11978.24f, -1705.43f, 32.40f, 0.0f},
    {-11972.51f, -1702.20f, 32.60f, 0.0f},
    {-11974.63f, -1694.00f, 32.40f, 0.0f},
    {-11970.75f, -1691.41f, 32.70f, 0.0f},
    {-11976.65f, -1683.74f, 32.80f, 0.0f},
    {-11988.19f, -1692.20f, 32.30f, 0.0f},
    {-11983.91f, -1696.76f, 32.30f, 0.0f},
    {-11989.18f, -1701.09f, 32.30f, 0.0f},
    {-11995.53f, -1697.08f, 32.30f, 0.0f},
    {-11992.38f, -1694.31f, 32.30f, 0.0f},
    {-12001.17f, -1688.04f, 32.30f, 0.0f}
};

class boss_venoxis : public CreatureScript
{
    public:
        boss_venoxis() : CreatureScript("boss_venoxis") { }
        

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new boss_venoxisAI(pCreature);
        }

        struct boss_venoxisAI : public BossAI
        {
            boss_venoxisAI(Creature* pCreature) : BossAI(pCreature, DATA_VENOXIS)
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
                me->SetReactState(REACT_AGGRESSIVE);
                me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, false);
                me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_ATTACK_ME, false);
            }

            void EnterCombat(Unit* /*who*/)
            {
                Talk(SAY_AGGRO);
                phase = 0;
                me->SetReactState(REACT_AGGRESSIVE);
                //me->SummonCreature(NPC_VENOMOUS_EFFUSION_STALKER, mazeleftPos[0]);
                events.RescheduleEvent(EVENT_TOXIC_LINK, 7000);
                events.RescheduleEvent(EVENT_WHISPER_OF_HETHISS, 6000);
                events.RescheduleEvent(EVENT_TRANSFORM, 35000);
                DoCastAOE(SPELL_WORD_OF_HETHISS);
                DoZoneInCombat();
                instance->SetBossState(DATA_VENOXIS, IN_PROGRESS);
            }

            void JustDied(Unit* /*killer*/)
            {
                _JustDied();
                Talk(SAY_DEATH);
            }

            void MovementInform (uint32 type, uint32 id)
            {
                if (id == POINT_DOWN && phase == 2)
                {
                    events.RescheduleEvent(EVENT_MOVE_UP, 400);
                }
                else if (id == POINT_UP && phase == 2)
                {
                    events.RescheduleEvent(EVENT_BLOODVENOM, 1000);
                }
                else if (id == POINT_WITHDRAWAL)
                {
                    me->RemoveAura(SPELL_BLESSING_OF_THE_SNAKE_GOD);
                    me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, false);
                    me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_ATTACK_ME, false);
                    DoCast(me, SPELL_VENOM_WITHDRAWAL);
                    me->SetReactState(REACT_AGGRESSIVE);
                    events.RescheduleEvent(EVENT_TOXIC_LINK, 17000);
                    events.RescheduleEvent(EVENT_WHISPER_OF_HETHISS, 16000);
                    events.RescheduleEvent(EVENT_TRANSFORM, 45000);
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
                        case EVENT_TOXIC_LINK:
                            DoCastAOE(SPELL_TOXIC_LINK);
                            events.RescheduleEvent(EVENT_TOXIC_LINK, urand(15000, 19000));
                            break;
                        case EVENT_WHISPER_OF_HETHISS:
                            //if (Unit* pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                                //DoCast(pTarget, SPELL_WHISPER_OF_HETHISS);
                            events.RescheduleEvent(EVENT_WHISPER_OF_HETHISS, 10000);
                            break;
                        case EVENT_TRANSFORM:
                            Talk(SAY_TRANSFORM);
                            phase = 1;
                            events.CancelEvent(EVENT_TOXIC_LINK);
                            events.CancelEvent(EVENT_WHISPER_OF_HETHISS);
                            DoCast(me, SPELL_BLESSING_OF_THE_SNAKE_GOD);
                            events.RescheduleEvent(EVENT_BREATH_OF_HETHISS, urand(6000, 9000));
                            events.RescheduleEvent(EVENT_POOL_OF_ACID_TEARS, urand(8000, 10000));
                            events.RescheduleEvent(EVENT_MOVE_DOWN, 40000);
                            break;
                        case EVENT_BREATH_OF_HETHISS:
                            DoCastAOE(SPELL_BREATH_OF_HETHISS);
                            events.RescheduleEvent(EVENT_BREATH_OF_HETHISS, 10000);
                            break;
                        case EVENT_POOL_OF_ACID_TEARS:
                            if (Unit* pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                                DoCast(pTarget, SPELL_POOL_OF_ACID_TEARS);
                            events.RescheduleEvent(EVENT_POOL_OF_ACID_TEARS, 7000);
                            break;
                        case EVENT_MOVE_DOWN:
                            events.CancelEvent(EVENT_BREATH_OF_HETHISS);
                            events.CancelEvent(EVENT_POOL_OF_ACID_TEARS);
                            phase = 2;
                            me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, true);
                            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_ATTACK_ME, true);
                            me->AttackStop();
                            me->SetReactState(REACT_PASSIVE);
                            me->GetMotionMaster()->MovePoint(POINT_DOWN, downPos);
                            break;
                        case EVENT_MOVE_UP:
                            me->GetMotionMaster()->Clear();
                            me->GetMotionMaster()->MovePoint(POINT_UP, upPos);
                            break;
                        case EVENT_BLOODVENOM:
                            Talk(SAY_BLOODVENOM);
                            DoCastAOE(SPELL_BLOODVENOM);
                            events.RescheduleEvent(EVENT_VENOM_WITHDRAWAL, 15500);
                            break;
                        case EVENT_VENOM_WITHDRAWAL:
                            phase = 0;
                            me->GetMotionMaster()->MovePoint(POINT_WITHDRAWAL, downPos);
                            break;
                    }
                }
            
                DoMeleeAttackIfReady();
            }
        };
};
class npc_venoxis_bloodvenom : public CreatureScript
{
    public:

        npc_venoxis_bloodvenom() : CreatureScript("npc_venoxis_bloodvenom") {}
        
        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_venoxis_bloodvenomAI(pCreature);
        }

        struct npc_venoxis_bloodvenomAI : public Scripted_NoMovementAI
        {
            npc_venoxis_bloodvenomAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature) 
            {
                me->SetSpeed(MOVE_RUN, 0.3f);
                me->SetReactState(REACT_PASSIVE);
                instance = pCreature->GetInstanceScript();
                DoCast(me, SPELL_BLOODVENOM_DUMMY, true);
            }
            
            InstanceScript* instance;
            bool bMove;
            uint32 moveTimer;

            void Reset()
            {
                bMove = false;
                moveTimer = 1000;
            }
            
            void IsSummonedBy(Unit* owner)
            {
                if (Creature* pVenoxis = me->FindNearestCreature(NPC_VENOXIS, 120.0f))
                    DoCast(pVenoxis, SPELL_VENOM_TOTEM_BEAM_CENTER);
            }

            void UpdateAI(uint32 diff)
            {
                if (!instance || instance->GetBossState(DATA_VENOXIS) != IN_PROGRESS)
                {
                    me->DespawnOrUnsummon();
                    return;
                }

                if (moveTimer <= diff && !bMove)
                {
                    if (me->GetOwner())
                    {
                        bMove = true;
                        DoCast(me, SPELL_BLOODVENOM_AURA, true);
                        me->GetMotionMaster()->MoveFollow(me->GetOwner(), 0.0f, 0.0f);
                    }
                }
                else
                    moveTimer -= diff;
            }
        };
};

class npc_venoxis_venomous_effusion_stalker : public CreatureScript
{
    public:

        npc_venoxis_venomous_effusion_stalker() : CreatureScript("npc_venoxis_venomous_effusion_stalker") {}
        
        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_venoxis_venomous_effusion_stalkerAI(pCreature);
        }

        struct npc_venoxis_venomous_effusion_stalkerAI : public Scripted_NoMovementAI
        {
            npc_venoxis_venomous_effusion_stalkerAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature) 
            {
                me->SetSpeed(MOVE_RUN, 0.3f);
                me->SetReactState(REACT_PASSIVE);
                instance = pCreature->GetInstanceScript();
                DoCast(me, SPELL_VENOMOUS_EFFUSION, true);
            }
            
            InstanceScript* instance;
            uint8 side;
            uint32 moveTimer;
            uint8 waypoint;

            void Reset()
            {
                side = (me->GetPositionY() < -1690.0f)? 0: 1;
                waypoint = 1;
                moveTimer = 1;
            }

            void MovementInform(uint32 type, uint32 id)
            {
                if (waypoint == 18)
                    me->DespawnOrUnsummon();
                else
                    moveTimer = 1;
            }
            
            void KilledUnit(Unit* victim)
            {
                Talk(SAY_KILL);
            }

            void JustSummoned(Creature* summon)
            {
                if (me->isInCombat())
                    DoZoneInCombat(summon);
            }

            void UpdateAI(uint32 diff)
            {
                if (!instance || instance->GetBossState(DATA_VENOXIS) != IN_PROGRESS)
                {
                    me->DespawnOrUnsummon();
                    return;
                }

                if (moveTimer)
                {
                    me->GetMotionMaster()->Clear();
                    me->GetMotionMaster()->MovePoint(0, mazeleftPos[waypoint]);
                    ++waypoint;
                    moveTimer = 0;
                }
            }
        };
};

class npc_venoxis_venomous_effusion : public CreatureScript
{
    public:

        npc_venoxis_venomous_effusion() : CreatureScript("npc_venoxis_venomous_effusion") {}
        
        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_venoxis_venomous_effusionAI(pCreature);
        }

        struct npc_venoxis_venomous_effusionAI : public Scripted_NoMovementAI
        {
            npc_venoxis_venomous_effusionAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature) 
            {
                me->SetReactState(REACT_PASSIVE);
                instance = pCreature->GetInstanceScript();
                //DoCast(me, SPELL_VENOMOUS_EFFUSION_DUMMY, true);
                //DoCast(me, SPELL_VENOMOUS_EFFUSION_AURA, true);
            }

            InstanceScript* instance;

            void UpdateAI(uint32 diff)
            {
                if (!instance || instance->GetBossState(DATA_VENOXIS) != IN_PROGRESS)
                {
                    me->DespawnOrUnsummon();
                    return;
                }
            }
        };
};

class npc_venoxis_venoxis_pool_of_acid_tears : public CreatureScript
{
    public:

        npc_venoxis_venoxis_pool_of_acid_tears() : CreatureScript("npc_venoxis_venoxis_pool_of_acid_tears") {}
        
        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_venoxis_venoxis_pool_of_acid_tearsAI(pCreature);
        }

        struct npc_venoxis_venoxis_pool_of_acid_tearsAI : public Scripted_NoMovementAI
        {
            npc_venoxis_venoxis_pool_of_acid_tearsAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature) 
            {
                me->SetReactState(REACT_PASSIVE);
                DoCast(me, SPELL_POOL_OF_ACID_TEARS_AURA, true);
            }
        };
};

class spell_venoxis_toxic_link : public SpellScriptLoader
{
    public:
        spell_venoxis_toxic_link() : SpellScriptLoader("spell_venoxis_toxic_link") { }

        class spell_venoxis_toxic_link_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_venoxis_toxic_link_SpellScript);
            
            bool Load()
            {
                bCanLink = false;
                target1 = NULL;
                target2 = NULL;
                return true;
            }

            void HandleDummy()
            {
                if (!GetCaster())
                    return;

                if (bCanLink &&target1 && target1->isAlive() && target1->IsInWorld() &&
                    target2 && target2->isAlive() && target2->IsInWorld())
                {
                    GetCaster()->CastSpell(target1, SPELL_TOXIC_LINK_AURA, true);
                    GetCaster()->CastSpell(target2, SPELL_TOXIC_LINK_AURA, true);
                }
            }
            
            void FilterTargets(std::list<WorldObject*>& targets)
            {
                if (targets.size() < 2)
                    bCanLink = false;
                else
                {
                    std::list<WorldObject*>::const_iterator itr = targets.begin();
                    target1 = (*itr)->ToUnit();
                    ++itr;
                    target2 = (*itr)->ToUnit();
                    bCanLink = true;
                }
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_venoxis_toxic_link_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
                AfterCast += SpellCastFn(spell_venoxis_toxic_link_SpellScript::HandleDummy);
            }

        private:
            bool bCanLink;
            Unit* target1;
            Unit* target2;
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_venoxis_toxic_link_SpellScript();
        }
};

class spell_venoxis_toxic_link_aura : public SpellScriptLoader
{
    public:
        spell_venoxis_toxic_link_aura() : SpellScriptLoader("spell_venoxis_toxic_link_aura") { }

        class spell_venoxis_toxic_link_aura_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_venoxis_toxic_link_aura_AuraScript);

            void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (GetCaster())
                    GetCaster()->CastSpell(GetCaster(), SPELL_TOXIC_EXPLOSION, true);
            }
            
            void PeriodicTick(AuraEffect const* aurEff)
            {
                if (!GetCaster() || !GetTarget())
                    return;

                 UnitList targets;
                 Trinity::AnyUnitHavingBuffInObjectRangeCheck u_check(GetCaster(), GetTarget(), 100, SPELL_TOXIC_LINK_AURA, true);
                 Trinity::UnitListSearcher<Trinity::AnyUnitHavingBuffInObjectRangeCheck> searcher(GetTarget(), targets, u_check);
                 Trinity::VisitNearbyObject(GetTarget(), 80, searcher);

                 if (targets.size() < 2)
                     GetTarget()->RemoveAura(SPELL_TOXIC_LINK_AURA);
                 else
                 {
                     for (UnitList::const_iterator itr = targets.begin(); itr != targets.end(); ++itr)
                         if ((*itr)->GetGUID() != GetTarget()->GetGUID())
                         {
                             if (!GetTarget()->IsWithinDistInMap((*itr), 30.0f))
                             {
                                 GetTarget()->RemoveAurasDueToSpell(SPELL_TOXIC_LINK_AURA);
                                 (*itr)->RemoveAurasDueToSpell(SPELL_TOXIC_LINK_AURA);
                             }
                             break;
                         }
                 }
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_venoxis_toxic_link_aura_AuraScript::PeriodicTick, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
                AfterEffectRemove += AuraEffectRemoveFn(spell_venoxis_toxic_link_aura_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_venoxis_toxic_link_aura_AuraScript();
        }
};

class ExactDistanceCheck
{
    public:
        ExactDistanceCheck(Unit* source, float dist) : _source(source), _dist(dist) {}

        bool operator()(WorldObject* unit)
        {
            return _source->GetExactDist2d(unit) > _dist;
        }

    private:
        Unit* _source;
        float _dist;
};

class spell_venoxis_pool_of_acid_tears_dmg : public SpellScriptLoader
{
    public:
        spell_venoxis_pool_of_acid_tears_dmg() : SpellScriptLoader("spell_venoxis_pool_of_acid_tears_dmg") { }

        class spell_venoxis_pool_of_acid_tears_dmg_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_venoxis_pool_of_acid_tears_dmg_SpellScript);

            void CorrectRange(std::list<WorldObject*>& targets)
            {
                targets.remove_if(ExactDistanceCheck(GetCaster(), 2.0f * GetCaster()->GetFloatValue(OBJECT_FIELD_SCALE)));
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_venoxis_pool_of_acid_tears_dmg_SpellScript::CorrectRange, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_venoxis_pool_of_acid_tears_dmg_SpellScript();
        }
};

void AddSC_boss_venoxis()
{
    new boss_venoxis();
    new npc_venoxis_venomous_effusion_stalker();
    new npc_venoxis_venomous_effusion();
    new npc_venoxis_bloodvenom();
    new npc_venoxis_venoxis_pool_of_acid_tears();
    new spell_venoxis_toxic_link();
    new spell_venoxis_toxic_link_aura();
    new spell_venoxis_pool_of_acid_tears_dmg();
}
