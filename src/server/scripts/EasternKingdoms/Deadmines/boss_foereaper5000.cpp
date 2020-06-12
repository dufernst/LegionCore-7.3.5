#include "deadmines.h"

enum ScriptTexts
{
    SAY_AGGRO     = 0,
    SAY_DEATH     = 1,
    SAY_KILL      = 2,
    SAY_SPELL1    = 3,
    SAY_SPELL2    = 4,
    SAY_SPELL3    = 5,
    SAY_SPELL4    = 6
};

enum Spells
{
    SPELL_OFF_LINE             = 88348,
    SPELL_REAPER_STRIKE        = 88490,
    SPELL_REAPER_STRIKE_H      = 91717,
    SPELL_SAFETY               = 88522,
    SPELL_SAFETY_H             = 91720,
    SPELL_HARVEST              = 88495,
    SPELL_HARVEST_AURA         = 88497,
    SPELL_HARVEST_DMG          = 88501,
    SPELL_HARVEST_DMG_H        = 91719,
    SPELL_HARVEST_SWEEP        = 88521,
    SPELL_HARVEST_SWEEP_H      = 91718,
    SPELL_OVERDRIVE            = 88481,
    SPELL_OVERDRIVE_DMG        = 88484,
    SPELL_OVERDRIVE_DMG_H      = 91716,
    SPELL_TARGET_BUNNY         = 71371 //rocket artillery
};

enum Adds
{
    NPC_TARGETING_BUNNY     = 47468,
    NPC_DEFIAS_REAPER       = 47403,
    NPC_MOLTEN_SLAG         = 49229,
    NPC_PROTOTYPE_REAPER    = 49208 // 87239 91731
};

enum Events
{
    EVENT_OVERDRIVE         = 1,
    EVENT_OVERDRIVE1        = 2,
    EVENT_REAPER_STRIKE     = 3,
    EVENT_HARVEST           = 4,
    EVENT_HARVEST1          = 5

};

const Position defiasreaperPos[3] =
{
    { -182.742f, -565.968f, 19.389f, 3.35f},
    { -228.675f, -565.752f, 19.389f, 5.98f},
    { -201.526f, -548.737f, 51.229f, 4.43f}
};

const Position moltenslagPos[4] =
{
    { -205.582f, -572.034f, 20.97f, 1.59f},
    { -199.143f, -579.843f, 20.97f, 6.16f},
    { -206.385f, -585.898f, 20.97f, 5.17f},
    { -212.704f, -579.072f, 20.97f, 3.09f}
};

class boss_foereaper5000 : public CreatureScript
{
    public:
        boss_foereaper5000() : CreatureScript("boss_foereaper5000") {}

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new boss_foereaper5000AI (pCreature);
        }

        struct boss_foereaper5000AI : public BossAI
        {
            boss_foereaper5000AI(Creature* pCreature) : BossAI(pCreature, DATA_FOEREAPER)
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
            }

            ObjectGuid harvestTargetGuid;
            bool bEnrage;

            void Reset() override
            {
                _Reset();

                bEnrage = false;
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;
                
                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                if (HealthBelowPct(30) && !bEnrage)
                {
                    Talk(SAY_SPELL4);
                    bEnrage = true;
                    DoCast(me, SPELL_SAFETY);
                    return;
                }

                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_OVERDRIVE:
                            DoCast(me, SPELL_OVERDRIVE);
                            Talk(SAY_SPELL3);
                            events.RescheduleEvent(EVENT_OVERDRIVE, urand(25000, 30000));
                            break;
                        case EVENT_REAPER_STRIKE:
                            DoCast(me->getVictim(), SPELL_REAPER_STRIKE);
                            events.RescheduleEvent(EVENT_REAPER_STRIKE, urand(7000, 10000));
                            break;
                        case EVENT_HARVEST:
                            if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                            {
                                DoCast(target, SPELL_HARVEST);
                                Talk(SAY_SPELL1);
                            }
                            events.RescheduleEvent(EVENT_HARVEST, 30000, 40000);
                            break;
                        }
                }

                DoMeleeAttackIfReady();
            }

            void MovementInform(uint32 type, uint32 id) override
            {
                if (type == POINT_MOTION_TYPE)
                {
                    switch (id)
                    {
                        case 1001:
                            DoCast(me, DUNGEON_MODE(SPELL_HARVEST_SWEEP, SPELL_HARVEST_SWEEP), true);
                            me->RemoveAurasDueToSpell(SPELL_HARVEST_AURA);
                            if(auto harvestTarget = ObjectAccessor::GetCreature(*me, harvestTargetGuid))
                                harvestTarget->DespawnOrUnsummon(1000);
                            break;
                    }
                }
            }

            void EnterCombat(Unit* /*who*/) override
            {
                Talk(SAY_AGGRO);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_STUNNED);
                events.RescheduleEvent(EVENT_REAPER_STRIKE, urand(5000, 8000));
                events.RescheduleEvent(EVENT_OVERDRIVE, urand(10000, 15000));
                events.RescheduleEvent(EVENT_HARVEST, urand(25000, 30000));
                DoZoneInCombat();
                instance->SetBossState(DATA_FOEREAPER, IN_PROGRESS);
            }

            void KilledUnit(Unit* /*victim*/) override
            {
                Talk(SAY_KILL);
            }

            void JustSummoned(Creature* summon) override
            {
                BossAI::JustSummoned(summon);
                if (summon->GetEntry()== NPC_TARGETING_BUNNY)
                {
                    harvestTargetGuid = summon->GetGUID();
                    Talk(SAY_SPELL2);
                    me->GetMotionMaster()->MovePoint(1001, summon->GetPositionX(), summon->GetPositionY(), summon->GetPositionZ());
                }
            }

            void JustDied(Unit* /*killer*/)
            {
                _JustDied();
                Talk(SAY_DEATH); 
            }
        };
};

class npc_foereaper_targeting_bunny: public CreatureScript
{
    public:
        npc_foereaper_targeting_bunny() : CreatureScript("npc_foereaper_targeting_bunny") {}
     
        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_foereaper_targeting_bunnyAI (pCreature);
        }
     
        struct npc_foereaper_targeting_bunnyAI : public Scripted_NoMovementAI
        {
            npc_foereaper_targeting_bunnyAI(Creature *c) : Scripted_NoMovementAI(c)
            {
                instance = c->GetInstanceScript();
            }
           
            InstanceScript* instance;

            void Reset() override
            {
                if (!instance)
                    return;

                DoCast(SPELL_TARGET_BUNNY);
            }
        };
};

void AddSC_boss_foereaper5000()
{
    new boss_foereaper5000();
    new npc_foereaper_targeting_bunny();
}