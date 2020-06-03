#include "halls_of_origination.h"

// Sun-Touched Spriteling 39370
// Sun-Touched Sprite 39369

enum Spells
{
    //Rajh
    SPELL_SUN_STRIKE                    = 73872, 
    SPELL_SUN_STRIKE_DMG                = 73874,
    SPELL_BLESSING_OF_THE_SUN           = 76352,
    SPELL_BLESSING_OF_THE_SUN_AURA      = 76355,
    SPELL_ZERO_POWER                    = 72242,
    SPELL_SUMMON_SOLAR_WINDS            = 74104,
    SPELL_SOLAR_WINDS_AURA              = 74107,
    SPELL_SOLAR_WINDS_DMG               = 74108,
    SPELL_SOLAR_WINDS_VISUAL            = 74109,
    SPELL_SOLAR_FIRE                    = 89131,
    SPELL_SOLAR_FIRE_DMG                = 89133
};

enum Texts
{
    SAY_DEATH    = 0,
    SAY_AGGRO    = 1,
    SAY_ENERGIZE = 2,
    SAY_KILL     = 3
};

const Position pointcenter = {-319.68f, 193.42f, 343.94f, 4.76f};

enum Events
{
    EVENT_SUN_STRIKE           = 1,
    EVENT_SUMMON_SOLAR_WINDS   = 2
};

enum Adds
{
    NPC_SOLAR_WINDS1            = 39634,
    NPC_SOLAR_WINDS2            = 39635, // 74109
    NPC_SOLAR_WINDS3            = 47922,
    NPC_SUN_TOUCHED_SPRITELING  = 39370, // 76159
    NPC_SUN_TOUCHED_SPRITE      = 39369, // 76158,
    NPC_ORB_OF_SUN              = 40835, // 73658
    NPC_INFERNO_LEAP            = 47040,
    NPC_BLAZING_INFERNO         = 40927
};

class boss_rajh : public CreatureScript
{
    public:
        boss_rajh() : CreatureScript("boss_rajh") {}

        CreatureAI* GetAI(Creature* creature) const
        {
            return GetAIForInstance<boss_rajhAI>(creature, HOScriptName);
        }

        struct boss_rajhAI : public BossAI
        {
            explicit boss_rajhAI(Creature* creature) : BossAI(creature, DATA_RAJH)
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

            uint8 phase;
            bool achieve;

            void Reset() override
            {
                _Reset();

                bool achieve = false;

                phase = 0;

                SetEquipmentSlots(false, 1728, 0, 0);

                DoCast(me, SPELL_ZERO_POWER, true);
                me->setPowerType(POWER_ENERGY);
                me->SetMaxPower(POWER_ENERGY, 100);
                me->SetPower(POWER_ENERGY, 100);
            }

            void KilledUnit(Unit* /*Killed*/) override
            {
                Talk(SAY_KILL);
            }

            void JustDied(Unit* /*Kill*/) override
            {
                _JustDied();
                Talk(SAY_DEATH);
            }

            bool HasAchieved()
            {
                return achieve;
            }

            void EnterCombat(Unit* /*Ent*/) override
            {
                Talk(SAY_AGGRO);
                
                events.ScheduleEvent(EVENT_SUN_STRIKE, urand(5000, 10000));
                events.ScheduleEvent(EVENT_SUMMON_SOLAR_WINDS, urand(10000, 15000));

                achieve = true;
                DoZoneInCombat();
                instance->SetBossState(DATA_RAJH, IN_PROGRESS);
            }

            void JustReachedHome() override
            {
                _JustReachedHome();
            }

            void MovementInform(uint32 type, uint32 id) override
            {
                if (type == POINT_MOTION_TYPE)
                {
                    switch (id)
                    {
                        case 1:
                            Talk(SAY_ENERGIZE);
                            DoCast(SPELL_BLESSING_OF_THE_SUN);
                            break;
                    }
                }
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                if (me->GetPower(POWER_ENERGY) < 1 && phase == 0)
                {
                    phase = 1;
                    events.Reset();
                    me->AttackStop();
                    me->SetReactState(REACT_PASSIVE);
                    me->GetMotionMaster()->MovePoint(1, pointcenter);
                    return;
                }
                else if (me->GetPower(POWER_ENERGY) > 99 && phase == 1)
                {
                    phase = 0;
                    achieve = false;
                    me->SetReactState(REACT_AGGRESSIVE);
                    events.ScheduleEvent(EVENT_SUN_STRIKE, urand(5000, 10000));
                    events.ScheduleEvent(EVENT_SUMMON_SOLAR_WINDS, urand(6000, 11000));

                    if (auto victim = me->getVictim())
                        me->GetMotionMaster()->MoveChase(victim);
                    return;
                }

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_SUN_STRIKE:
                            if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                                DoCast(target, SPELL_SUN_STRIKE, false);

                            events.ScheduleEvent(EVENT_SUN_STRIKE, urand(20000, 25000));
                            break;
                        case EVENT_SUMMON_SOLAR_WINDS:
                            if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                                DoCast(target, SPELL_SUMMON_SOLAR_WINDS, false);

                            events.ScheduleEvent(EVENT_SUMMON_SOLAR_WINDS, urand(20000, 25000));
                            break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };
};

struct npc_rajh_solar_wind : public ScriptedAI
{
    explicit npc_rajh_solar_wind(Creature* creature) : ScriptedAI(creature)
    {
        me->SetReactState(REACT_PASSIVE);
    }

    void Reset() override
    {
        DoCast(me, SPELL_SOLAR_WINDS_VISUAL, true);
        DoCast(me, SPELL_SOLAR_WINDS_AURA, true);
    }
};

typedef boss_rajh::boss_rajhAI RajhAI;

class achievement_sun_of_a : public AchievementCriteriaScript
{
    public:
        achievement_sun_of_a() : AchievementCriteriaScript("achievement_sun_of_a") {}

        bool OnCheck(Player* source, Unit* target)
        {
            if (!target)
                return false;

            if (RajhAI* rajhAI = CAST_AI(RajhAI, target->GetAI()))
                return rajhAI->HasAchieved();

            return false;
        }
};

void AddSC_boss_rajh()
{
    new boss_rajh();
    new achievement_sun_of_a();
    RegisterCreatureAI(npc_rajh_solar_wind);
}
