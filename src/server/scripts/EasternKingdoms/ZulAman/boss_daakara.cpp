#include "zulaman.h"

enum Scripttexts
{
    SAY_BEAR        = 0,
    SAY_DEATH       = 1,
    SAY_DRAGONHAWK  = 2,
    SAY_EAGLE       = 3,
    SAY_FIRE        = 4,
    SAY_KILL        = 5,
    SAY_LYNX        = 6,
    SAY_AGGRO       = 7,
};

enum Spells
{
    SPELL_WHIRLWIND                 = 17207,
    SPELL_GRIEVOUS_THROW            = 43093,

    SPELL_CREEPING_PARALYSIS        = 43095,
    SPELL_OVERPOWER                 = 43456,
    SPELL_SURGE                     = 42402,
    
    SPELL_SUMMON_AMANI_LYNX         = 97686,
    SPELL_LYNX_RUSH                 = 43153,
    SPELL_CLAW_RAGE_CHARGE          = 42583,
    SPELL_CLAW_RAGE                 = 43149,

    SPELL_FLAME_BREATH              = 43140,
    SPELL_FLAME_WHIRL               = 43213,
    SPELL_PILLAR_OF_FLAME_SUMMON    = 43216,
    SPELL_PILLAR_OF_FLAME_VISUAL    = 43541,
    SPELL_PILLAR_OF_FLAME_AURA      = 43218,

    //SPELL_ENERGY_STORM              = 43983,
    //SPELL_ZAP_INFORM                = 42577,
    //SPELL_ZAP_DAMAGE                = 43137,
    //SPELL_SUMMON_VORTEX             = 43112,

    SPELL_SWEEPING_WINDS            = 97647,
    SPELL_FEATHER_VORTEX_VISUAL     = 97835,
    SPELL_FEATHER_VORTEX_PASSIVE    = 43120,
    SPELL_LIGHTNING_TOTEM           = 97930,

    //SPELL_SPIRIT_AURA               = 42466,
    //SPELL_SIPHON_SOUL               = 43501,

    SPELL_SHAPE_OF_THE_BEAR         = 42594,
    SPELL_SHAPE_OF_THE_EAGLE        = 42606,
    SPELL_SHAPE_OF_THE_LYNX         = 42607,
    SPELL_SHAPE_OF_THE_DRAGONHAWK   = 42608,

    SPELL_BERSERK                   = 45078,

    SPELL_FERAL_SWIPE               = 43357,
};

enum Events
{
    EVENT_WHIRLWIND             = 1,
    EVENT_GRIEVOUS_THROW        = 2,
    EVENT_CREEPING_PARALYSIS    = 3,
    EVENT_OVERPOWER             = 4,
    EVENT_SURGE                 = 5,
    EVENT_LYNX_RUSH             = 6,
    EVENT_CLAW_RAGE             = 7,
    EVENT_SWEEPING_WINDS        = 8,
    EVENT_FLAME_BREATH          = 9,
    EVENT_FLAME_WHIRL           = 10,
    EVENT_PILLAR_OF_FLAME       = 11,
    EVENT_BERSERK               = 12,
    EVENT_CONTINUE              = 13,
    EVENT_FERAL_SWIPE           = 14,
    EVENT_LIGHTNING_TOTEM       = 15,
    EVENT_CHECK_PLAYERS         = 16,
};

enum Adds
{
    NPC_PILLAR_OF_FLAME     = 24187,
    NPC_AMANI_LYNX          = 52839,
    NPC_FEATHER_VORTEX      = 24136,
};

enum Actions
{
    ACTION_MOVE_TO_PLAYER   = 1,
};

#define CENTER_X 120.148811f
#define CENTER_Y 703.713684f
#define CENTER_Z 45.111477f
#define MAX_X 136.199707f
#define MIN_X 104.217331f
#define MAX_Y 715.623901f
#define MIN_Y 697.285217f

class boss_daakara : public CreatureScript
{
    public:
        boss_daakara() : CreatureScript("boss_daakara") { }
        
        CreatureAI* GetAI(Creature* pCreature) const
        {
            return GetInstanceAI<boss_daakaraAI>(pCreature);
        }

        struct boss_daakaraAI : public BossAI
        {
            boss_daakaraAI(Creature* pCreature) : BossAI(pCreature, DATA_DAAKARA)
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
                bAchieve = true;
            }

            void Reset()
            {
                _Reset();

                phase = 0;
                bEnrage = false;
                bAchieve = true;

                me->SetReactState(REACT_AGGRESSIVE);
                me->SetVirtualItem(0, 33975);
                //me->SetUInt32Value(UNIT_VIRTUAL_ITEM_INFO, 218172674);
                //me->SetByteValue(UNIT_FIELD_BYTES_2, 0, SHEATH_STATE_MELEE);
            }

            void EnterCombat(Unit* /*who*/)
            {
                Talk(SAY_AGGRO);

                phase = 0;
                bEnrage = false;
                bAchieve = true;

                me->SetReactState(REACT_AGGRESSIVE);

                events.RescheduleEvent(EVENT_BERSERK, 660000);
                events.RescheduleEvent(EVENT_GRIEVOUS_THROW, urand(3000, 8000));
                events.RescheduleEvent(EVENT_WHIRLWIND, urand(6000, 8000));
                events.RescheduleEvent(EVENT_CHECK_PLAYERS, 5000);
                DoZoneInCombat();
                instance->SetBossState(DATA_DAAKARA, IN_PROGRESS);
            }

            void KilledUnit(Unit* /*victim*/)
            {
                Talk(SAY_KILL);
            }

            void JustDied(Unit* /*killer*/)
            {
                _JustDied();
                Talk(SAY_DEATH);
            }

            void SpellHit(Unit* attacker, const SpellInfo* spellInfo)
            {
                if (spellInfo->HasEffect(SPELL_EFFECT_ATTACK_ME) ||
                    spellInfo->HasAura(SPELL_AURA_MOD_TAUNT))
                    me->RemoveAurasDueToSpell(SPELL_CLAW_RAGE);
            }

            bool AllowAchieve()
            {
                return bAchieve;
            }

            void EnterPhase(uint8 form)
            {
                DoResetThreat();
                me->SetVirtualItem(0, 0);
                
                switch (form)
                {
                    case 0:
                        Talk(SAY_BEAR);
                        me->RemoveAurasDueToSpell(SPELL_WHIRLWIND);
                        DoCast(me, SPELL_SHAPE_OF_THE_BEAR, true);
                        events.RescheduleEvent(EVENT_SURGE, urand(4000, 10000));
                        events.RescheduleEvent(EVENT_OVERPOWER, urand(2000, 9000));
                        events.RescheduleEvent(EVENT_CREEPING_PARALYSIS, urand(10000, 15000));
                        break;
                    case 1:
                        Talk(SAY_LYNX);
                        me->RemoveAurasDueToSpell(SPELL_WHIRLWIND);
                        DoCast(me, SPELL_SHAPE_OF_THE_LYNX, true);
                        DoCast(me, SPELL_SUMMON_AMANI_LYNX, true);
                        events.RescheduleEvent(EVENT_LYNX_RUSH, urand(5000, 12000));
                        events.RescheduleEvent(EVENT_CLAW_RAGE, urand(8000, 15000));
                        break;
                    case 2:
                        Talk(SAY_DRAGONHAWK);
                        me->AttackStop();
                        me->SetReactState(REACT_PASSIVE);
                        //me->NearTeleportTo(CENTER_X, CENTER_Y, CENTER_Z, 0.0f);
                        me->RemoveAurasDueToSpell(SPELL_SHAPE_OF_THE_LYNX);
                        me->RemoveAurasDueToSpell(SPELL_SHAPE_OF_THE_BEAR);
                        DoCast(me, SPELL_SHAPE_OF_THE_DRAGONHAWK, true);
                        events.RescheduleEvent(EVENT_FLAME_BREATH, urand(5000, 12000));
                        events.RescheduleEvent(EVENT_FLAME_WHIRL, urand(3000, 5000));
                        events.RescheduleEvent(EVENT_PILLAR_OF_FLAME, urand(12000, 17000));
                        break;
                    case 3:
                        Talk(SAY_EAGLE);
                        me->AttackStop();
                        me->SetReactState(REACT_PASSIVE);
                        //me->NearTeleportTo(CENTER_X, CENTER_Y, CENTER_Z, 0.0f);
                        me->RemoveAurasDueToSpell(SPELL_SHAPE_OF_THE_LYNX);
                        me->RemoveAurasDueToSpell(SPELL_SHAPE_OF_THE_BEAR);
                        DoCast(me, SPELL_SHAPE_OF_THE_EAGLE, true);
                        std::list<Unit*> targets;
                        SelectTargetList(targets, 5, SELECT_TARGET_RANDOM, 0.0f, true);
                        if (targets.size() > 0)
                        {
                            for (std::list<Unit*>::const_iterator itr = targets.begin(); itr != targets.end(); ++itr)
                            {
                                if (Creature* pCyclone = me->SummonCreature(NPC_FEATHER_VORTEX, (*itr)->GetPositionX(), (*itr)->GetPositionY(), (*itr)->GetPositionZ(), (*itr)->GetOrientation()))
                                    pCyclone->AI()->SetGUID((*itr)->GetGUID());
                            }
                        }
                        events.RescheduleEvent(EVENT_SWEEPING_WINDS, 5000);
                        events.RescheduleEvent(EVENT_LIGHTNING_TOTEM, urand(10000, 13000));
                        break;
                    }
            }

            void UpdateAI(uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                if (me->HealthBelowPct(80) && phase == 0)
                {
                    phase++;
                    events.CancelEvent(EVENT_WHIRLWIND); 
                    events.CancelEvent(EVENT_GRIEVOUS_THROW);
                    EnterPhase(urand(0, 1));
                    return;
                }
                else if (me->HealthBelowPct(40) && phase == 1)
                {
                    phase++;
                    events.CancelEvent(EVENT_SURGE);
                    events.CancelEvent(EVENT_CREEPING_PARALYSIS);
                    events.CancelEvent(EVENT_OVERPOWER);
                    events.CancelEvent(EVENT_LYNX_RUSH);
                    events.CancelEvent(EVENT_CLAW_RAGE);
                    //EnterPhase(urand(2, 3));
                    EnterPhase(2);
                    return;
                }

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                if (uint32 eventId = events.ExecuteEvent())
                {
                     switch (eventId)
                     {
                        case EVENT_BERSERK:
                            DoCast(me, SPELL_BERSERK);
                            break;
                        case EVENT_CHECK_PLAYERS:
                        {
                            Map::PlayerList const &plrList = instance->instance->GetPlayers();
                            if (!plrList.isEmpty())
                            {
                                for (Map::PlayerList::const_iterator itr = plrList.begin(); itr != plrList.end(); ++itr)
                                    if (Player* pPlayer = itr->getSource())
                                        if (pPlayer->GetPositionX() < MIN_X ||
                                            pPlayer->GetPositionX() > MAX_X ||
                                            pPlayer->GetPositionY() < MIN_Y ||
                                            pPlayer->GetPositionY() > MAX_Y)
                                        {
                                            bAchieve = false;
                                            break;
                                        }
                            }
                            if (bAchieve)
                                events.RescheduleEvent(EVENT_CHECK_PLAYERS, 5000);
                            break;
                        }
                        case EVENT_WHIRLWIND:
                            DoCastAOE(SPELL_WHIRLWIND);
                            events.RescheduleEvent(EVENT_WHIRLWIND, urand(12000, 15000));
                            break;
                        case EVENT_GRIEVOUS_THROW:
                            if (Unit* pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                                DoCast(pTarget, SPELL_GRIEVOUS_THROW);
                            events.RescheduleEvent(EVENT_GRIEVOUS_THROW, urand(9000, 11000));
                            break;
                        case EVENT_CREEPING_PARALYSIS:
                            DoCastAOE(SPELL_CREEPING_PARALYSIS);
                            events.RescheduleEvent(EVENT_CREEPING_PARALYSIS, urand(15000, 18000));
                            break;
                        case EVENT_SURGE:
                            if (Unit* pTarget = SelectTarget(SELECT_TARGET_FARTHEST, 0, 0.0f, true))
                                DoCast(pTarget, SPELL_SURGE);
                            events.RescheduleEvent(EVENT_SURGE, urand(10000, 15000));
                            break;
                        case EVENT_OVERPOWER:
                            DoCastVictim(SPELL_OVERPOWER);
                            events.RescheduleEvent(EVENT_OVERPOWER, 10000);
                            break;
                        case EVENT_LYNX_RUSH:
                            DoCastVictim(SPELL_LYNX_RUSH);
                            events.RescheduleEvent(EVENT_LYNX_RUSH, 10000);
                            break;
                        case EVENT_CLAW_RAGE:
                            if (Unit* pTarget = SelectTarget(SELECT_TARGET_RANDOM, 1, 0.0f, true))
                                DoCast(pTarget, SPELL_CLAW_RAGE_CHARGE);
                            events.RescheduleEvent(EVENT_CLAW_RAGE, urand(20000, 25000));
                            break;
                        case EVENT_FLAME_WHIRL:
                            DoCastAOE(SPELL_FLAME_WHIRL);
                            events.RescheduleEvent(EVENT_FLAME_WHIRL, 12000);
                            break;
                        case EVENT_FLAME_BREATH:
                            DoCastAOE(SPELL_FLAME_BREATH);
                            events.RescheduleEvent(EVENT_FLAME_BREATH, 15000);
                            break;
                        case EVENT_PILLAR_OF_FLAME:
                            if (Unit* pTarget = SelectTarget(SELECT_TARGET_RANDOM,0, 0.0f, true))
                                DoCast(pTarget, SPELL_PILLAR_OF_FLAME_SUMMON);
                            events.RescheduleEvent(EVENT_PILLAR_OF_FLAME, urand(12000, 18000));
                            break;
                        case EVENT_SWEEPING_WINDS:
                            DoCast(me, SPELL_SWEEPING_WINDS);
                            events.RescheduleEvent(EVENT_SWEEPING_WINDS, 8500);
                            break;
                        case EVENT_LIGHTNING_TOTEM:
                            DoCast(me, SPELL_LIGHTNING_TOTEM);
                            events.RescheduleEvent(EVENT_LIGHTNING_TOTEM, 15000);
                            break;
                     }
                }

                DoMeleeAttackIfReady();
            }

        private:
            ObjectGuid ClawTargetGUID;
            bool bEnrage;
            uint8 phase;
            bool bAchieve;
        };
};

class npc_daakara_vortex : public CreatureScript
{
    public:

        npc_daakara_vortex() : CreatureScript("npc_daakara_vortex") {}
        
        CreatureAI* GetAI(Creature* pCreature) const
        {
            return GetInstanceAI<npc_daakara_vortexAI>(pCreature);
        }

        struct npc_daakara_vortexAI : public Scripted_NoMovementAI
        {
            npc_daakara_vortexAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature) 
            {
                DoCast(me, SPELL_FEATHER_VORTEX_VISUAL, true);
                me->SetReactState(REACT_PASSIVE);
                startTimer = 2000;
                victimGUID.Clear();
            }

            bool bStart;
            uint32 startTimer;

            ObjectGuid victimGUID;

            void Reset()
            {
            }
            
            void SetGUID(ObjectGuid const& guid, int32 data)
            {
                victimGUID = guid;
            }

            void DoAction(const int32 action)
            {
                if (action == ACTION_MOVE_TO_PLAYER)
                    if (Unit* pTarget = ObjectAccessor::GetUnit(*me, victimGUID))
                        me->GetMotionMaster()->MoveChase(pTarget);

            }

            void UpdateAI(uint32 diff)
            {
                if (startTimer <= diff && !bStart)
                {
                    bStart = true;
                    DoCast(me, SPELL_FEATHER_VORTEX_PASSIVE, true);
                }
                else
                    startTimer -= diff;
            }
        };
};

class npc_daakara_pillar_of_flame : public CreatureScript
{
    public:

        npc_daakara_pillar_of_flame() : CreatureScript("npc_daakara_pillar_of_flame") {}
        
        CreatureAI* GetAI(Creature* pCreature) const
        {
            return GetInstanceAI<npc_daakara_pillar_of_flameAI>(pCreature);
        }

        struct npc_daakara_pillar_of_flameAI : public Scripted_NoMovementAI
        {
            npc_daakara_pillar_of_flameAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature) 
            {
                me->SetReactState(REACT_PASSIVE);
                DoCast(me, SPELL_PILLAR_OF_FLAME_VISUAL, true);
                bExplode = false;
                explodeTimer = 2000;
            }
            
            bool bExplode;
            uint32 explodeTimer;

            void Reset()
            {
            }

            void UpdateAI(uint32 diff)
            {
                if (explodeTimer <= diff && !bExplode)
                {
                    bExplode = true;
                    me->RemoveAllAuras();
                    DoCast(me, SPELL_PILLAR_OF_FLAME_AURA, true);
                }
                else
                    explodeTimer -= diff;
            }
        };
};

class npc_daakara_amani_lynx : public CreatureScript
{
    public:

        npc_daakara_amani_lynx() : CreatureScript("npc_daakara_amani_lynx") {}
        
        CreatureAI* GetAI(Creature* pCreature) const
        {
            return GetInstanceAI<npc_daakara_amani_lynxAI>(pCreature);
        }

        struct npc_daakara_amani_lynxAI : public ScriptedAI
        {
            npc_daakara_amani_lynxAI(Creature* pCreature) : ScriptedAI(pCreature) 
            {
            }
            
            EventMap events;
            
            void Reset()
            {
                events.Reset();
            }
            
            void EnterCombat(Unit* who)
            {
                events.RescheduleEvent(EVENT_FERAL_SWIPE, urand(6000, 9000));
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
                        case EVENT_FERAL_SWIPE:
                            DoCastVictim(SPELL_FERAL_SWIPE);
                            events.RescheduleEvent(EVENT_FERAL_SWIPE, urand(7000, 12000));
                            break;
                     }
                }

                DoMeleeAttackIfReady();
            }

        };
};

class spell_daakara_claw_rage_charge : public SpellScriptLoader
{
    public:
        spell_daakara_claw_rage_charge() : SpellScriptLoader("spell_daakara_claw_rage_charge") { }

        class spell_daakara_claw_rage_charge_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_daakara_claw_rage_charge_SpellScript);

            void HandleCharge(SpellEffIndex effIndex)
            {
                if (GetCaster() && GetHitUnit())
                {
                    if (GetCaster()->IsAIEnabled)
                        GetCaster()->GetAI()->AttackStart(GetHitUnit());
                    GetCaster()->CastSpell(GetHitUnit(), SPELL_CLAW_RAGE, true);
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_daakara_claw_rage_charge_SpellScript::HandleCharge, EFFECT_0, SPELL_EFFECT_CHARGE);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_daakara_claw_rage_charge_SpellScript();
        }
};

class spell_daakara_sweeping_winds : public SpellScriptLoader
{
    public:
        spell_daakara_sweeping_winds() : SpellScriptLoader("spell_daakara_sweeping_winds") { }

        class spell_daakara_sweeping_winds_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_daakara_sweeping_winds_SpellScript);

            void HandleDummy(SpellEffIndex effIndex)
            {
                if (GetHitUnit() && GetHitUnit()->GetEntry() == NPC_FEATHER_VORTEX)
                    GetHitUnit()->GetAI()->DoAction(ACTION_MOVE_TO_PLAYER);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_daakara_sweeping_winds_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_daakara_sweeping_winds_SpellScript();
        }
};

typedef boss_daakara::boss_daakaraAI DaakaraAI;

class achievement_ring_out : public AchievementCriteriaScript
{
    public:
        achievement_ring_out() : AchievementCriteriaScript("achievement_ring_out") { }

        bool OnCheck(Player* source, Unit* target)
        {
            if (!target)
                return false;

            if (DaakaraAI* daakaraAI = CAST_AI(DaakaraAI, target->GetAI()))
                return daakaraAI->AllowAchieve();

            return false;
        }
};

void AddSC_boss_daakara()
{
    new boss_daakara();
    new npc_daakara_vortex();
    new npc_daakara_pillar_of_flame();
    new npc_daakara_amani_lynx();
    new spell_daakara_claw_rage_charge();
    new spell_daakara_sweeping_winds();
    new achievement_ring_out();
}

