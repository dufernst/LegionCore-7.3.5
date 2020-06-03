#include "deadmines.h"
#include "LFGMgr.h"
#include "Group.h"

enum ScriptedTexts
{
    SAY_AGGRO   = 0,
    SAY_DEATH   = 1,
    SAY_KILL    = 2,
    SAY_ADDS    = 3,
    SAY_BOMB_1  = 4,
    SAY_BOMB_2  = 5,
    SAY_INTRO_1 = 8,
    SAY_INTRO_2 = 9,
    SAY_INTRO_3 = 10,
    SAY_INTRO_4 = 11,
    SAY_INTRO_5 = 12
};

enum Spells
{
    SPELL_DEADLY_BLADES         = 92622,
    SPELL_BACKSLASH_TARGETING   = 92620,
    SPELL_BACKSLASH             = 92619,
    SPELL_DEFLECTION            = 92614,
    SPELL_VENGEANCE_OF_VANCLEEF = 95542,
    SPELL_POWDER_EXPLOSION      = 96283,
    SPELL_SITTING               = 89279,
    SPELL_NOXIOUS_CONCOCTION    = 92100,
    SPELL_VANESSA_ACHIEVEMENT   = 95654,
    SPELL_SUMMON_ENFORCER       = 92616,
    SPELL_SUMMON_SHADOWGUARD    = 92618,
    SPELL_SUMMON_BLOOD_WIZARD   = 92617,

    // Defias Enforcer
    SPELL_CHARGE                = 90928,
    SPELL_BLOODBATH             = 90925,
    SPELL_RECKLESSNESS          = 90929,

    // Defias Blood Wizard
    SPELL_BLOODWASH             = 90946,
    SPELL_BLOODBOLT             = 90938,
    SPELL_RAGEZONE              = 90932
};

enum Adds
{
    NPC_DEFIAS_ENFORCER             = 49850,
    NPC_DEFIAS_SHADOWGUARD          = 49852,
    NPC_DEFIAS_BLOOD_WIZARD         = 49854,
    NPC_ROPE                        = 49550, // 95527
    // event
    NPC_VANESSA_EVENT               = 49671, // 48143 69676
    NPC_VANESSA_TRAP_BUNNY          = 49454,
    NPC_GLUBTOK_EVENT               = 49670,
    NPC_HELIX_EVENT                 = 49674,
    NPC_CHATTERING_HORROR           = 49495,
    NPC_DARKWEB_DEVOURER            = 49494,
    NPC_FOEREAPER_EVENT             = 49681,
    NPC_VANESSA_LIGHTNING_PLATTER   = 49520,
    NPC_VANESSA_LIGHTNING_STALKER   = 49521,
    NPC_RIPSNARL_EVENT              = 49682,
    NPC_ENRAGED_WORGEN              = 49532,
    NPC_ERIK_HARRINGTON             = 49535, // 2
    NPC_EMME_HARRINGTON             = 49534, // 1
    NPC_CALISSA_HARRINGTON          = 49536, // 3 92608
    NPC_JAMES_HARRINGTON            = 49539
};

enum Events
{
    EVENT_INTRO_1       = 1,
    EVENT_INTRO_2       = 2,
    EVENT_INTRO_3       = 3,
    EVENT_INTRO_4       = 4,
    EVENT_INTRO_5       = 5,
    EVENT_INTRO_6       = 6,
    EVENT_INTRO_7       = 7,
    EVENT_ADDS_1        = 8,
    EVENT_ADDS_2        = 9,
    EVENT_ADDS_3        = 10,
    EVENT_BACKSLASH     = 11,
    EVENT_DEFLECTION    = 12,
    EVENT_CONTINUE      = 13,
    EVENT_VENGEANCE     = 14,
    EVENT_DEADLY_BLADES = 15
};

#define ACHIEVEMENT_EVENT 27527

const Position addsPos[3] = 
{
    {-67.115807f, -829.708740f, 40.959011f, 1.344201f},
    {-69.161903f, -814.125977f, 40.722912f, 1.611237f},
    {-57.057121f, -816.067200f, 41.852272f, 5.565719f}
};

class boss_vanessa_vancleef : public CreatureScript
{
    public:
        boss_vanessa_vancleef() : CreatureScript("boss_vanessa_vancleef") {}

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new boss_vanessa_vancleefAI (pCreature);
        }

        struct boss_vanessa_vancleefAI : public BossAI
        {
            boss_vanessa_vancleefAI(Creature* pCreature) : BossAI(pCreature, DATA_VANESSA)
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
                me->SetReactState(REACT_DEFENSIVE);
            }

            void Reset() override
            {
                me->SetReactState(REACT_DEFENSIVE);
                _Reset();
                if (instance->GetData(DATA_VANESSA_EVENT) == DONE)
                    me->SetVisible(true);
                else
                    me->SetVisible(false);
            }

            void InitializeAI()
            {
                if (!instance || static_cast<InstanceMap*>(me->GetMap())->GetScriptId() != sObjectMgr->GetScriptId(DMScriptName))
                    me->IsAIEnabled = false;
                else if (!me->isDead())
                    Reset();
            }

            void EnterCombat(Unit* /*who*/)  override
            {
                Talk(SAY_AGGRO);
                DoCast(SPELL_VANESSA_ACHIEVEMENT);

                events.RescheduleEvent(EVENT_BACKSLASH, urand(10000, 15000));
                events.RescheduleEvent(EVENT_DEFLECTION, 10000);
                events.RescheduleEvent(EVENT_DEADLY_BLADES, 12000);
                events.RescheduleEvent(EVENT_ADDS_1, 9000);

                DoZoneInCombat();
                instance->SetBossState(DATA_VANESSA, IN_PROGRESS);
            }

            void DamageTaken(Unit* done_by, uint32& damage, DamageEffectType dmgType) override
            {
                if (me->HealthBelowPctDamaged(25, damage))
                {
                    events.RescheduleEvent(EVENT_VENGEANCE, 4000);
                }
            }

            void KilledUnit(Unit* victim) override
            {
                if (victim && victim->IsPlayer())
                    Talk(SAY_KILL);
            }

            void JustDied(Unit* /*killer*/) override
            {
                Talk(SAY_DEATH);

                _JustDied();

                Map::PlayerList const& players = me->GetMap()->GetPlayers();
                if (!players.isEmpty())
                {
                    Player* pPlayer = players.begin()->getSource();
                    if (pPlayer && pPlayer->GetGroup())
                        sLFGMgr->FinishDungeon(pPlayer->GetGroup()->GetGUID(), 326);
                }
            }

            void UpdateAI(uint32 diff) override
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
                        case EVENT_BACKSLASH:
                            DoCast(me, SPELL_DEADLY_BLADES);
                            events.RescheduleEvent(EVENT_CONTINUE, 6000);
                            break;
                        case EVENT_CONTINUE:
                            me->GetMotionMaster()->MoveChase(me->getVictim());
                            break;
                        case EVENT_ADDS_1:
                            if ((me->GetHealth() * 100) / me->GetMaxHealth() > 50)
                            {
                                Talk(SAY_ADDS);
                                me->SummonCreature(NPC_DEFIAS_ENFORCER, addsPos[1], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                                events.RescheduleEvent(EVENT_ADDS_2, 15000);
                            }
                            break;
                        case EVENT_ADDS_2:
                            if ((me->GetHealth() * 100) / me->GetMaxHealth() > 50)
                            {
                                me->SummonCreature(NPC_DEFIAS_SHADOWGUARD, addsPos[0], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                                events.RescheduleEvent(EVENT_ADDS_3, 15000);
                            }
                            break;
                        case EVENT_ADDS_3:
                            if ((me->GetHealth() * 100) / me->GetMaxHealth() > 50)
                            {
                                me->SummonCreature(NPC_DEFIAS_BLOOD_WIZARD, addsPos[2], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                                events.RescheduleEvent(EVENT_ADDS_1, 15000);
                            }
                        case EVENT_VENGEANCE:
                            me->AddAura(SPELL_VENGEANCE_OF_VANCLEEF, me);
                            break;
                        case EVENT_DEFLECTION:
                            if (HealthAbovePct(25))
                            {
                                DoCast(me, SPELL_DEFLECTION);
                                events.RescheduleEvent(EVENT_DEFLECTION, 50000);
                            }
                            break;
                        case EVENT_DEADLY_BLADES:
                            DoCast(me, SPELL_DEADLY_BLADES);
                            events.RescheduleEvent(EVENT_DEADLY_BLADES, 35000);
                            break;
                        default:
                            break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };
};

class npc_vanessa_vancleef_a_note_from_vanessa : public CreatureScript
{
    public:
        npc_vanessa_vancleef_a_note_from_vanessa() : CreatureScript("npc_vanessa_vancleef_a_note_from_vanessa") {}
     
        bool OnGossipHello(Player* pPlayer, Creature* pCreature)
        {
            InstanceScript* instance = pCreature->GetInstanceScript();
            if (!instance)
                return true;
            if (instance->GetBossState(DATA_CAPTAIN) != DONE)
                return true;
            if (instance->GetData(DATA_VANESSA_EVENT) != NOT_STARTED)
                return true;

            pCreature->SummonCreature(NPC_VANESSA_SITTING, vanessaPos);
            instance->SetData(DATA_VANESSA_EVENT, IN_PROGRESS);

            pCreature->RemoveFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
            pCreature->DespawnOrUnsummon(1000);
            return true;
        }
};

class npc_vanessa_vancleef_vanessa_sitting : public CreatureScript
{
    public:
        npc_vanessa_vancleef_vanessa_sitting() : CreatureScript("npc_vanessa_vancleef_vanessa_sitting") {}

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_vanessa_vancleef_vanessa_sittingAI (pCreature);
        }

        struct npc_vanessa_vancleef_vanessa_sittingAI : public BossAI
        {
            npc_vanessa_vancleef_vanessa_sittingAI(Creature* pCreature) : BossAI(pCreature, DATA_VANESSA)
            {
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                me->setActive(true);
                instance = me->GetInstanceScript();
            }

            void InitializeAI()
            {
                if (!instance || static_cast<InstanceMap*>(me->GetMap())->GetScriptId() != sObjectMgr->GetScriptId(DMScriptName))
                    me->IsAIEnabled = false;
                else if (!me->isDead())
                    Reset();
            }

            void Reset() override
            {
                events.Reset();
            }

            void IsSummonedBy(Unit* /*owner*/) override
            {
                events.RescheduleEvent(EVENT_INTRO_1, 1000);
            }

            void UpdateAI(uint32 diff) override
            {
                events.Update(diff);
                
                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_INTRO_1:
                            Talk(SAY_INTRO_1);
                            events.RescheduleEvent(EVENT_INTRO_2, 5000);
                            break;
                        case EVENT_INTRO_2:
                            Talk(SAY_INTRO_2);
                            events.RescheduleEvent(EVENT_INTRO_3, 8000);
                            break;
                        case EVENT_INTRO_3:
                            Talk(SAY_INTRO_3);
                            events.RescheduleEvent(EVENT_INTRO_4, 8000);
                            break;
                        case EVENT_INTRO_4:
                            Talk(SAY_INTRO_4);
                            events.RescheduleEvent(EVENT_INTRO_5, 5000);
                            break;
                        case EVENT_INTRO_5:
                            Talk(SAY_INTRO_5);
                            DoCast(SPELL_NOXIOUS_CONCOCTION);
                            me->RemoveAura(SPELL_SITTING);
                            events.RescheduleEvent(EVENT_INTRO_6, 5000);
                            break;
                        case EVENT_INTRO_6:
                            me->GetMotionMaster()->MoveJump(centershipPos.GetPositionX(), centershipPos.GetPositionY(), centershipPos.GetPositionZ(), 5.f, 10.f);
                            events.RescheduleEvent(EVENT_INTRO_7, 5000);
                            break;
                        case EVENT_INTRO_7:
                            if (instance)
                                instance->SetData(DATA_VANESSA_EVENT, DONE);
                            if (auto pVanessa = me->FindNearestCreature(NPC_VANESSA_VANCLEEF, 40.0f))
                                pVanessa->SetVisible(true);
                            
                            instance->DoStartTimedAchievement(CRITERIA_TIMED_TYPE_EVENT2, ACHIEVEMENT_EVENT);
                            me->DespawnOrUnsummon(1000);
                            break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        private:
            InstanceScript* instance;
            EventMap events;
        };
};

class npc_vanessa_vancleef_defias_enforcer : public CreatureScript
{
    public:
        npc_vanessa_vancleef_defias_enforcer() : CreatureScript("npc_vanessa_vancleef_defias_enforcer") {}

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_vanessa_vancleef_defias_enforcerAI (pCreature);
        }

        struct npc_vanessa_vancleef_defias_enforcerAI : public BossAI
        {
            npc_vanessa_vancleef_defias_enforcerAI(Creature* pCreature) : BossAI(pCreature, DATA_VANESSA)
            {
                me->setActive(true);
                instance = me->GetInstanceScript();
            }

            void Reset() override
            {
                events.Reset();
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                DoMeleeAttackIfReady();
            }

        private:
            InstanceScript* instance;
            EventMap events;
        };
};

class spell_vanessa_vancleef_backslash_targeting : public SpellScriptLoader
{
    public:
        spell_vanessa_vancleef_backslash_targeting() : SpellScriptLoader("spell_vanessa_vancleef_backslash_targeting") { }

        class spell_vanessa_vancleef_backslash_targeting_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_vanessa_vancleef_backslash_targeting_SpellScript);


            void HandleScript(SpellEffIndex effIndex)
            {
                if (!GetCaster() || !GetHitUnit())
                    return;

                GetCaster()->CastSpell(GetHitUnit(), SPELL_BACKSLASH, true);
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_vanessa_vancleef_backslash_targeting_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_vanessa_vancleef_backslash_targeting_SpellScript();
        }
};

void AddSC_boss_vanessa_vancleef()
{
    new boss_vanessa_vancleef();
    new npc_vanessa_vancleef_a_note_from_vanessa();
    new npc_vanessa_vancleef_vanessa_sitting();
    new spell_vanessa_vancleef_backslash_targeting();
}