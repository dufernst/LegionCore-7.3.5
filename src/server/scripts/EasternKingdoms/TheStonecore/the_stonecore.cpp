#include "the_stonecore.h"

enum Spells
{
    // Crystalspawn Giant
    SPELL_QUAKE              = 81008,
    H_SPELL_QUAKE            = 92631,

    // Imp
    SPELL_FELL_FIREBALL      = 80344,
    H_SPELL_FELL_FIREBALL    = 92638, 

    // Millhouse Manastorm
    SPELL_BLUR               = 81216,
    SPELL_MILL_FEAR          = 81442,
    SPELL_FROSTBOLT_VOLLEY   = 81440,           
    H_SPELL_FROSTBOLT_VOLLEY = 92642,
    SPELL_IMPENDING_DOOM     = 86830,
    SPELL_SHADOW_BOLT        = 81439,           
    H_SPELL_SHADOW_BOLT      = 92641,
    SPELL_SHADOWFURY         = 81441,
    H_SPELL_SHADOWFURY       = 92644,
    SPELL_TIGULE             = 81220,

    // Stonecore Berserker
    SPELL_SCHARGE            = 81574,
    SPELL_SPINNING_SLASH     = 81568,

    // Stonecore Bruiser
    SPELL_BODY_SLAM          = 80180,
    SPELL_SHOCKWAVE          = 80195,
    H_SPELL_SHOCKWAVE        = 92640,

    // Stonecore Earthshaper 
    SPELL_DUST_STORM         = 81463,
    SPELL_FORCE_OF_EARTH     = 81459,
    SPELL_GROUND_SHOCK       = 81530,       
    H_SPELL_GROUND_SHOCK     = 92628,
    SPELL_LAVA_BURST         = 81576,
    H_SPELL_LAVA_BURST       = 92626, 

    // Stonecore Flayer 
    SPELL_FLAY               = 79922,

    // Stonecore Magmalord (
    SPELL_IGNITE             = 80151,
    H_SPELL_IGNITE           = 92636,
    SPELL_MAGMA_ERUPTION     = 80038,

    // Stonecore Rift Conjurer 
    SPELL_DEMON_PORTAL       = 80308,
    SPELL_SHADOWBOLT         = 80279,             
    H_SPELL_SHADOWBOLT       = 92637,

    //Stonecore Sentry
    
    // Stonecore Warbringer 
    SPELL_CLEAVE             = 15496,
    SPELL_RAGE               = 80158,
};

enum eEvents
{
    EVENT_NONE,
    EVENT_QUAKE,
    EVENT_FELL_FIREBALL,
    EVENT_BLUR,
    EVENT_MILL_FEAR,
    EVENT_FROSTBOLT_VOLLEY,
    EVENT_IMPENDING_DOOM,
    EVENT_SHADOW_BOLT,
    EVENT_SHADOWFURY,
    EVENT_TIGULE,
    EVENT_ROCK_BORE,
    EVENT_SCHARGE,
    EVENT_SPINNING_SLASH,
    EVENT_BODY_SLAM,
    EVENT_SHOCKWAVE,
    EVENT_DUST_STORM,
    EVENT_FORCE_OF_EARTH,
    EVENT_GROUND_SHOCK,
    EVENT_LAVA_BURST,
    EVENT_FLAY,
    EVENT_IGNITE,
    EVENT_MAGMA_ERUPTION,
    EVENT_DEMON_PORTAL,
    EVENT_SHADOWBOLT,
    EVENT_CLEAVE,
    EVENT_RAGE,
};

class mob_crystalspawn_giant : public CreatureScript
{
public:
    mob_crystalspawn_giant() : CreatureScript("mob_crystalspawn_giant") { }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new mob_crystalspawn_giantAI(pCreature);
    }

    struct mob_crystalspawn_giantAI : public ScriptedAI
    {
        mob_crystalspawn_giantAI(Creature *c) : ScriptedAI(c)
        {
        }

        EventMap events;

        void Reset()
        {
            events.Reset();
        }

        void EnterCombat(Unit* /*who*/)
        {
            events.RescheduleEvent(EVENT_QUAKE, 5000 + rand()%5000);
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
                switch(eventId)
                {
                    case EVENT_QUAKE:
                        DoCast(me->getVictim(), SPELL_QUAKE);
                        events.RescheduleEvent(EVENT_QUAKE, 5000 + rand()%5000);
                        return;
                }
            }

            DoMeleeAttackIfReady();
        }
    };

};

class mob_impp : public CreatureScript
{
public:
    mob_impp() : CreatureScript("mob_impp") { }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new mob_imppAI(pCreature);
    }

    struct mob_imppAI : public ScriptedAI
    {
        mob_imppAI(Creature *c) : ScriptedAI(c)
        {
        }

        EventMap events;

        void Reset()
        {
            events.Reset();
        }

        void EnterCombat(Unit* /*who*/)
        {
            events.RescheduleEvent(EVENT_FELL_FIREBALL, 1000);
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
                switch(eventId)
                {
                    case EVENT_FELL_FIREBALL:
                        if (Unit *pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0))
                            DoCast(pTarget, SPELL_FELL_FIREBALL);
                        events.RescheduleEvent(EVENT_FELL_FIREBALL, 1000);
                        return;
                }
            }

            DoMeleeAttackIfReady();
        }
    };

};

enum ScriptTexts
{
    SAY_EVENT_1    = 0,
    SAY_EVENT_2    = 1,
    SAY_EVENT_3 = 2,
    SAY_DOOM    = 3,
    SAY_DEATH    = 4,
};

const Position millhousemanastormscPos[3] = 
{
    {987.67f, 882.45f, 303.37f, 2.07f},
    {1075.72f, 862.74f, 291.48f, 2.86f},
    {1151.45f, 885.74f, 284.96f, 3.36f}
};

class mob_millhouse_manastorm : public CreatureScript
{
public:
    mob_millhouse_manastorm() : CreatureScript("mob_millhouse_manastorm") { }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new mob_millhouse_manastormAI(pCreature);
    }

    struct mob_millhouse_manastormAI : public ScriptedAI
    {
        mob_millhouse_manastormAI(Creature *c) : ScriptedAI(c)
        {
            stage = 0;
        }

        EventMap events;
        uint8 stage;

        void Reset()
        {
            events.Reset();
        }

        void EnterCombat(Unit* /*who*/)
        {
            events.RescheduleEvent(EVENT_MILL_FEAR, 10000);
            events.RescheduleEvent(EVENT_FROSTBOLT_VOLLEY, 7000 + rand()%10000);
            events.RescheduleEvent(EVENT_SHADOW_BOLT, 1000);
            events.RescheduleEvent(EVENT_SHADOWFURY, 5000 + rand()%15000);
        }

        /*void JustReachedHome()
        {
            me->RemoveAurasDueToSpell(SPELL_BLUR);
            if (stage == 3)
            {
                Talk(SAY_DOOM);
                events.RescheduleEvent(EVENT_IMPENDING_DOOM, 5000);
                DoCast(SPELL_IMPENDING_DOOM);
            }
        }*/

        void JustDied(Unit* killer)
        {
            Talk(SAY_DEATH);
        }

        /*void MovementInform(uint32 type, uint32 id)
        {
            if (type == POINT_MOTION_TYPE)
            {
                switch (id)
                {
                case 1:
                case 2:
                case 3:
                    me->SetReactState(REACT_AGGRESSIVE);
                    EnterEvadeMode();
                    break;
                }
            }
        }*/

        /*void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            if (me->HealthBelowPct(50) && !me->HasAura(SPELL_BLUR))
            {
                switch (stage)
                {
                case 0:
                    stage = 1;
                    me->SetHomePosition(millhousemanastormscPos[0]);
                    me->RemoveAllAuras();
                    me->SetHealth(me->GetMaxHealth());
                    DoCast(me, SPELL_BLUR);
                    me->SetReactState(REACT_PASSIVE);
                    me->GetMotionMaster()->MovePoint(1, millhousemanastormscPos[0]);
                    Talk(SAY_EVENT_1);
                    break;
                case 1:
                    stage = 2;
                    me->SetHomePosition(millhousemanastormscPos[1]);
                    me->RemoveAllAuras();
                    me->SetHealth(me->GetMaxHealth());
                    DoCast(me, SPELL_BLUR);
                    me->SetReactState(REACT_PASSIVE);
                    me->GetMotionMaster()->MovePoint(2, millhousemanastormscPos[1]);
                    Talk(SAY_EVENT_2);
                    break;
                case 2:
                    stage = 3;
                    me->SetHomePosition(millhousemanastormscPos[2]);
                    me->RemoveAllAuras();
                    me->SetHealth(me->GetMaxHealth());
                    DoCast(me, SPELL_BLUR);
                    me->SetReactState(REACT_PASSIVE);
                    me->GetMotionMaster()->MovePoint(3, millhousemanastormscPos[2]);
                    Talk(SAY_EVENT_3);
                    break;
                }
            }
        }*/

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch(eventId)
                {
                    case EVENT_IMPENDING_DOOM:
                        me->Kill(me);
                    break;
                    case EVENT_MILL_FEAR:
                        if (Unit *pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0))
                            DoCast(pTarget, SPELL_MILL_FEAR);
                        events.RescheduleEvent(EVENT_MILL_FEAR, 10000);
                        return;
                    case EVENT_SHADOW_BOLT:
                        DoCast(me->getVictim(), SPELL_SHADOW_BOLT);
                        events.RescheduleEvent(EVENT_SHADOWBOLT, 1000);
                        return;
                    case EVENT_FROSTBOLT_VOLLEY:
                        if (Unit *pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0))
                            DoCast(pTarget, SPELL_FROSTBOLT_VOLLEY);
                        events.RescheduleEvent(EVENT_FROSTBOLT_VOLLEY, rand()%15000);
                        return;
                    case EVENT_SHADOWFURY:
                        if (Unit *pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0))
                            DoCast(pTarget, SPELL_SHADOWFURY);
                        events.RescheduleEvent(SPELL_SHADOWFURY, 5000 + rand()%15000);
                        return;
                }
            }

            DoMeleeAttackIfReady();
        }
    };

};

class npc_stonecore_instance_portal: public CreatureScript
{
public:
    npc_stonecore_instance_portal() : CreatureScript("npc_stonecore_instance_portal") {}

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_stonecore_instance_portal_AI(pCreature);
    }

    struct npc_stonecore_instance_portal_AI : public CreatureAI
    {
       npc_stonecore_instance_portal_AI(Creature* creature) : CreatureAI(creature) {}

        void OnSpellClick(Unit* clicker)
        {
            if (InstanceScript* instance = me->GetInstanceScript())
                if (instance->GetBossState(DATA_SLABHIDE) != DONE)
                    return;

            if (me->GetEntry() == 51396)
                clicker->NearTeleportTo(1313.197f, 1236.351f, 246.957f, 4.733236f, false);
            else if (me->GetEntry() == 51397)
                clicker->NearTeleportTo(853.575f, 999.710f, 317.326f, 4.591864f, false);
        }

        void UpdateAI(uint32 diff) { }
    };
};

void AddSC_the_stonecore()
{
    new mob_crystalspawn_giant();
    new mob_impp();
    new mob_millhouse_manastorm();
    new npc_stonecore_instance_portal();
}
