#include "dragon_soul.h"

enum Adds
{
    NPC_HARBRINGER_OF_TWILIGHT      = 55969,
    NPC_HARBRINGER_OF_DESTRUCTION   = 55967,
    NPC_ARCANE_WARDEN               = 56141,
    NPC_FORCE_OF_DESTRUCTION        = 56143,
    NPC_TIME_WARDEN_1               = 56142,
    NPC_PORTENT_OF_TWILIGHT         = 56144,
    NPC_TIME_WARDEN_2               = 57474,
    NPC_CHAMPION_OF_TIME            = 55913,
    NPC_DREAM_WARDEN                = 56140,
    NPC_LIFE_WARDEN_1               = 56139,
    NPC_LIFE_WARDEN_2               = 57473,
    NPC_CHAMPION_OF_LIFE            = 55911,
    NPC_CHAMPION_OF_MAGIC           = 55912,
    NPC_CHAMPION_OF_EMERALD_DREAM   = 55914,
    NPC_FACELESS_DESTROYER          = 57746,
    NPC_FACELESS_CORRUPTOR          = 57749,
    NPC_RUIN_TENTACLE               = 57751,
    NPC_DREAM_WARDEN_1              = 56140,
    NPC_DREAM_WARDEN_2              = 57475,

    NPC_ANCIENT_WATER_LORD          = 57160,
    NPC_EARTHEN_DESTROYER           = 57158,
    NPC_EARTHEN_SOLDIER             = 57159,
    NPC_TWILIGHT_SIEGE_CAPTAIN      = 57280,
    NPC_TWILIGHT_PORTAL             = 57231,
    
    // deathwing 56173 (boss)
    // deathwing 57962 (near boss)
};

enum Menus
{
    GOSSIP_MENU_ULTRAXION_START = 13322
};

enum Spells
{
    // Ancient Water Lord
    SPELL_DRENCHED                  = 107801,
    SPELL_FLOOD_AOE                 = 107796,
    SPELL_FLOOD                     = 107797,
    SPELL_FLOOD_CHANNEL             = 107791,

    // Earthen Destroyer
    SPELL_BOULDER_SMASH_AOE         = 107596,
    SPELL_BOULDER_SMASH             = 107597,
    SPELL_DUST_STORM                = 107682,

    // Earthen Soldier
    SPELL_ZERO_POWER                = 78725,
    SPELL_SHADOW_BOLT               = 95440,
    SPELL_TWILIGHT_CORRUPTION       = 107852,
    SPELL_TWILIGHT_RAGE             = 107872,

    // Twilight Siege Captain
    SPELL_TWILIGHT_SUBMISSION       = 108183,
    SPELL_TWILIGHT_VOLLEY           = 108172,
    SPELL_TWILIGHT_PORTAL_BEAM      = 108096,

    // Crimson Globule
    SPELL_CRIMSON_BLOOD_OF_SHUMA    = 110750,
    SPELL_SEARING_BLOOD             = 108218,

    // Acidic Globule
    SPELL_ACIDIC_BLOOD_OF_SHUMA     = 110743,
    SPELL_DIGESTIVE_ACID            = 108419,

    // Glowing Globule
    SPELL_GLOWING_BLOOD_OF_SHUMA_1  = 110753, // dummy
    SPELL_GLOWING_BLOOD_OF_SHUMA_2  = 108221, // haste

    // Dark Globule
    SPELL_BLACK_BLOOD_OF_SHUMA      = 110746,
    SPELL_PSYCHIC_SLICE             = 105671,

    // Shadowed Globule
    SPELL_SHADOWED_BLOOD_OF_SHUMA   = 110748,
    SPELL_DEEP_CORRUPTION           = 108220,
    SPELL_DEEP_CORRUPTION_AURA      = 109389,
    SPELL_DEEP_CORRUPTION_DMG       = 109390,

    // Cobalt Globule
    SPELL_COBALT_BLOOD_OF_SHUMA     = 110747,
    SPELL_MANA_VOID                 = 108223,
    SPELL_MANA_VOID_BURN            = 108222,
    SPELL_MANA_VOID_DUMMY           = 108224, // calculate power
    SPELL_MANA_DIFFUSION            = 108228,

    // Flail of Go'rath
    SPELL_SLUDGE_SPEW               = 110102,
    SPELL_WILD_FLAIL                = 109199,
    SPELL_BLOOD_OF_GORATH_DUMMY_1   = 109352,

    // Claw of Go'rath
    SPELL_OOZE_SPIT                 = 109396,
    SPELL_TENTACLE_TOSS_AOE_1       = 109197, // select target for picking up
    SPELL_TENTACLE_TOSS_SCRIPT_1    = 109233, // pick up
    SPELL_TENTECLE_TOSS_VEHICLE     = 109214,
    SPELL_TENTACLE_TOSS_AOE_2       = 109237, // select target to toss
    SPELL_TENTACLE_TOSS_SUMMON      = 109238,
    SPELL_TENTACLE_TOSS_SCRIPT_2    = 109243,
    SPELL_TENTACLE_TOSS_FORCE       = 109241,
    SPELL_TENTACLE_TOSS_JUMP        = 109240,
    SPELL_TENTACLE_TOSS_DMG         = 109258,
    SPELL_BLOOD_OF_GORATH_DUMMY_2   = 109365,

    // Eye of Go'rath
    SPELL_SHADOW_GAZE               = 109391,
    SPELL_BLOOD_OF_GORATH_DUMMY_3   = 103932,

    // Deathwing (Ultraxion event)
    SPELL_MOLTEN_METEOR             = 108832,

    // Twilight Assaulter
    SPELL_TWLIGHT_FLAMES_CHANNEL    = 105655,
    SPELL_TWILIGHT_FLAMES_DMG       = 105579,
    SPELL_TWILIGHT_FIREBALL         = 105555,
    SPELL_TWILIGHT_BREATH           = 105858,

    SPELL_CHARGE_DRAGON_SOUL_LIFE   = 108242,
    SPELL_CHARGE_DRAGON_SOUL_MAGIC  = 108243,
    SPELL_CHARGE_DRAGON_SOUL_EARTH  = 108471,
    SPELL_CHARGE_DRAGON_SOUL_TIME   = 108472,
    SPELL_CHARGE_DRAGON_SOUL_DREAMS = 108473,
};

enum Events
{
    // Ancient Water Lord
    EVENT_DRENCHED              = 1,
    EVENT_FLOOD                 = 2,

    // Earthen Destroyer
    EVENT_BOULDER_SMASH         = 3,
    EVENT_DUST_STORM            = 4,

    // Earthen Soldier
    EVENT_SHADOW_BOLT           = 5,
    EVENT_TWILIGHT_CORRUPTION   = 6,

    // Twilight Siege Captain
    EVENT_TWILIGHT_VOLLEY       = 7,
    EVENT_TWILIGHT_SUBMISSION   = 8,

    // Twilight Portal
    EVENT_CHECK_PLAYERS         = 9,

    // Crimson Globule
    EVENT_SEARING_BLOOD         = 10,

    // Acidic Globule
    EVENT_DIGESTIVE_ACID        = 11,

    // Dark Globule
    EVENT_PSYCHIC_SLICE         = 12,

    // Shadowed Globule
    EVENT_DEEP_CORRUPTION       = 13,

    // Cobalt Globule
    EVENT_MANA_VOID             = 14,

    // Flail of Go'rath
    EVENT_SLUDGE_SPEW           = 15,
    EVENT_WILD_FLAIL            = 16,

    // Claw of Go'rath
    EVENT_OOZE_SPIT             = 16,
    EVENT_TENTACLE_TOSS         = 17,

    // Eye of Go'rath
    EVENT_SHADOW_GAZE           = 18,

    // Thrall
    EVENT_TALK_ULTRAXION_WIN_1  = 19,
    EVENT_TALK_ULTRAXION_WIN_2  = 20,
    EVENT_SPAWN_SHIP            = 21,
    EVENT_SPAWN_NPC             = 22,

    // Dragon Soul Events (after Hagara)
    EVENT_AFTER_HAGARA          = 23,
    EVENT_DRAGON_SOUL_1         = 24,
    EVENT_DRAGON_SOUL_2         = 25,
    EVENT_DRAGON_SOUL_3         = 26,
    EVENT_DRAGON_SOUL_4         = 27,
    EVENT_DRAGON_SOUL_5         = 28,
    EVENT_DRAGON_SOUL_6         = 29,
    EVENT_DRAGON_SOUL_7         = 30,
    EVENT_DRAGON_SOUL_8         = 31,
    EVENT_DRAGON_SOUL_9         = 32,
    EVENT_DRAGON_SOUL_10        = 33,
    EVENT_DRAGON_SOUL_11        = 34,

    // Ultraxion Trash events
    EVENT_SPAWN_DRAGONS         = 35,
    EVENT_DEATHWING_INTRO_1     = 36,
    EVENT_DEATHWING_INTRO_2     = 37,
    EVENT_DEATHWING_INTRO_3     = 38,
    EVENT_DEATHWING_INTRO_4     = 39,
    EVENT_DEATHWING_INTRO_5     = 40,
    
    EVENT_DEATHWING_OOC_VISUAL  = 41,
    EVENT_DEATHWING_OOC_CAST_1  = 42,
    EVENT_DEATHWING_OOC_CAST_2  = 43,
    EVENT_DEATHWING_OOC_CAST_3  = 44,
    EVENT_DEATHWING_OOC_RESET   = 45,

    EVENT_DRAGONS_INTRO_1       = 46,
    EVENT_DRAGONS_INTRO_2       = 47,
    EVENT_DRAGONS_INTRO_3       = 48,
    
    EVENT_TRASH_WIPE_CHECK      = 49,
    
    EVENT_DEATHWING_DESPAWN     = 50,
    EVENT_SPAWN_ULTRAXION       = 51,
    EVENT_ULTRAXION_NEAR        = 52,

    EVENT_LOAD_EVENT            = 53,
    EVENT_LOAD_TRASH_1          = 54,
    EVENT_LOAD_TRASH_2          = 55,

    EVENT_NEXT_ASSAULTER        = 56,
    EVENT_ASSAULTER_VISUAL      = 57,
    EVENT_ASSAULTER_BREATH      = 58,

    // open portal to eye of eternity
    EVENT_OPEN_PORTAL_START     = 59,
    EVENT_OPEN_PORTAL_1         = 60,
    EVENT_OPEN_PORTAL_2         = 61,
    EVENT_OPEN_PORTAL_3         = 62,
    EVENT_OPEN_PORTAL_4         = 63,
    EVENT_OPEN_PORTAL_5         = 64,
    EVENT_OPEN_PORTAL_6         = 65,
    EVENT_OPEN_PORTAL_7         = 66,
    EVENT_OPEN_PORTAL_8         = 67,

    EVENT_CHARGE_DRAGON_SOUL    = 68,

    EVENT_EMOTE_CHANNEL_VIS     = 69,
};

enum Misc
{
    POINT_OOC_1     = 1,
    POINT_OOC_2     = 2,
    POINT_OOC_3     = 3,
    POINT_EVENT_1   = 4,
    POINT_LAST      = 5,
    POINT_ASSAULTER = 6,
};

const Position DeathwingEventPos = {-1648.428f, -2380.605f, 387.681f, 0.0f}; 
const Position DeathwingDespawnPos = {-1417.52f, -2193.82f, 374.776f, 0.0f}; 

const Position DeathwingOOCVisual[3] =
{
    {-1671.63f, -2349.82f, 376.255f, 0.0f},
    {-1705.99f, -2457.97f, 385.585f, 0.0f},
    {-1786.06f, -2297.38f, 376.207f, 0.0f}
};

class npc_dragon_soul_ancient_water_lord : public CreatureScript
{
    public:
        npc_dragon_soul_ancient_water_lord() : CreatureScript("npc_dragon_soul_ancient_water_lord") { }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return GetInstanceAI<npc_dragon_soul_ancient_water_lordAI>(pCreature);
        }

        struct npc_dragon_soul_ancient_water_lordAI : public ScriptedAI
        {
            npc_dragon_soul_ancient_water_lordAI(Creature* pCreature) : ScriptedAI(pCreature)
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

            EventMap events;

            void Reset()
            {
                events.Reset();
                events.ScheduleEvent(EVENT_EMOTE_CHANNEL_VIS, 1000);
            }

            void EnterCombat(Unit* /*who*/)
            {
                me->SetUInt32Value(UNIT_FIELD_EMOTE_STATE, 0);
                events.ScheduleEvent(EVENT_FLOOD, urand(8000, 12000));
                events.ScheduleEvent(EVENT_DRENCHED, urand(3000, 6000));
            }

            void UpdateAI(uint32 diff)
            {
                if (!UpdateVictim())
                {
                    events.Update(diff);
                    if (events.ExecuteEvent() == EVENT_EMOTE_CHANNEL_VIS)
                        if (me->FindNearestCreature(NPC_TWILIGHT_PORTAL, 25.0f))
                            me->SetUInt32Value(UNIT_FIELD_EMOTE_STATE, EMOTE_STATE_SPELL_CHANNEL_OMNI);
                    return;
                }

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;
                
                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_DRENCHED:
                            if (Unit* pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                                DoCast(pTarget, SPELL_DRENCHED);
                            events.ScheduleEvent(EVENT_DRENCHED, urand(13000, 15000));
                            break;
                        case EVENT_FLOOD:
                            DoCastAOE(SPELL_FLOOD_AOE);
                            events.ScheduleEvent(EVENT_FLOOD, urand(18000, 25000));
                            break;
                        default:
                            break;
                    }
                }
                
                DoMeleeAttackIfReady();
            }
        };
};

class npc_dragon_soul_earthen_destroyer : public CreatureScript
{
    public:
        npc_dragon_soul_earthen_destroyer() : CreatureScript("npc_dragon_soul_earthen_destroyer") { }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return GetInstanceAI<npc_dragon_soul_earthen_destroyerAI>(pCreature);
        }

        struct npc_dragon_soul_earthen_destroyerAI : public ScriptedAI
        {
            npc_dragon_soul_earthen_destroyerAI(Creature* pCreature) : ScriptedAI(pCreature)
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

            EventMap events;

            void Reset()
            {
                events.Reset();
                events.ScheduleEvent(EVENT_EMOTE_CHANNEL_VIS, 1000);
            }

            void EnterCombat(Unit* /*who*/)
            {
                me->SetUInt32Value(UNIT_FIELD_EMOTE_STATE, 0);
                events.ScheduleEvent(EVENT_BOULDER_SMASH, urand(3000, 5000));
                events.ScheduleEvent(EVENT_DUST_STORM, urand(7000, 11000));
            }

            void UpdateAI(uint32 diff)
            {
                if (!UpdateVictim())
                {
                    events.Update(diff);
                    if (events.ExecuteEvent() == EVENT_EMOTE_CHANNEL_VIS)
                        if (me->FindNearestCreature(NPC_TWILIGHT_PORTAL, 25.0f))
                            me->SetUInt32Value(UNIT_FIELD_EMOTE_STATE, EMOTE_STATE_SPELL_CHANNEL_OMNI);
                    return;
                }

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;
                
                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_BOULDER_SMASH:
                            DoCastAOE(SPELL_BOULDER_SMASH_AOE);
                            events.ScheduleEvent(EVENT_BOULDER_SMASH, urand(6000, 7000));
                            break;
                        case EVENT_DUST_STORM:
                            DoCast(me, SPELL_DUST_STORM);
                            events.ScheduleEvent(EVENT_DUST_STORM, urand(15000, 20000));
                            break;
                        default:
                            break;
                    }
                }
                
                DoMeleeAttackIfReady();
            }
        };
};

class npc_dragon_soul_earthen_soldier : public CreatureScript
{
    public:
        npc_dragon_soul_earthen_soldier() : CreatureScript("npc_dragon_soul_earthen_soldier") { }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return GetInstanceAI<npc_dragon_soul_earthen_soldierAI>(pCreature);
        }

        struct npc_dragon_soul_earthen_soldierAI : public ScriptedAI
        {
            npc_dragon_soul_earthen_soldierAI(Creature* pCreature) : ScriptedAI(pCreature)
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

            EventMap events;

            void Reset()
            {
                DoCast(me, SPELL_ZERO_POWER, true);
                me->setPowerType(POWER_ENERGY);
                me->SetMaxPower(POWER_ENERGY, 0);
                me->SetPower(POWER_ENERGY, 0);
                events.Reset();
            }

            void EnterCombat(Unit* /*who*/)
            {
                events.ScheduleEvent(EVENT_SHADOW_BOLT, urand(3000, 5000));
                events.ScheduleEvent(EVENT_TWILIGHT_CORRUPTION, urand(6000, 7000));
            }

            void UpdateAI(uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                if (me->GetPower(POWER_ENERGY) == 100)
                {
                    DoCast(me, SPELL_TWILIGHT_RAGE);
                    return;
                }
                
                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_SHADOW_BOLT:
                            if (Unit* pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                                DoCast(pTarget, SPELL_SHADOW_BOLT);
                            events.ScheduleEvent(EVENT_SHADOW_BOLT, urand(6000, 9000));
                            break;
                        case EVENT_TWILIGHT_CORRUPTION:
                            DoCast(me, SPELL_TWILIGHT_CORRUPTION);
                            events.ScheduleEvent(EVENT_TWILIGHT_CORRUPTION, urand(12000, 14000));
                            break;
                        default:
                            break;
                    }
                }
                
                DoMeleeAttackIfReady();
            }
        };
};

class npc_dragon_soul_twilight_siege_captain : public CreatureScript
{
    public:
        npc_dragon_soul_twilight_siege_captain() : CreatureScript("npc_dragon_soul_twilight_siege_captain") { }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return GetInstanceAI<npc_dragon_soul_twilight_siege_captainAI>(pCreature);
        }

        struct npc_dragon_soul_twilight_siege_captainAI : public ScriptedAI
        {
            npc_dragon_soul_twilight_siege_captainAI(Creature* pCreature) : ScriptedAI(pCreature)
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

            EventMap events;

            void Reset()
            {
                events.Reset();
                if (me->FindNearestCreature(NPC_TWILIGHT_PORTAL, 20.0f))
                    DoCast(SPELL_TWILIGHT_PORTAL_BEAM);
            }

            void EnterCombat(Unit* /*who*/)
            {
                me->InterruptNonMeleeSpells(true);
                events.ScheduleEvent(EVENT_TWILIGHT_VOLLEY, urand(3000, 5000));
                events.ScheduleEvent(EVENT_TWILIGHT_SUBMISSION, urand(10000, 12000));
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
                        case EVENT_TWILIGHT_VOLLEY:
                            DoCastAOE(SPELL_TWILIGHT_VOLLEY);
                            events.ScheduleEvent(EVENT_TWILIGHT_VOLLEY, urand(5000, 10000));
                            break;
                        case EVENT_TWILIGHT_SUBMISSION:
                            if (Unit* pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                                DoCast(pTarget, SPELL_TWILIGHT_SUBMISSION);
                            events.ScheduleEvent(EVENT_TWILIGHT_SUBMISSION, urand(7000, 10000));
                            break;
                        default:
                            break;
                    }
                }
                
                DoMeleeAttackIfReady();
            }
        };
};

class npc_dragon_soul_twilight_portal : public CreatureScript
{
    public:
        npc_dragon_soul_twilight_portal() : CreatureScript("npc_dragon_soul_twilight_portal") { }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return GetInstanceAI<npc_dragon_soul_twilight_portalAI>(pCreature);
        }

        struct npc_dragon_soul_twilight_portalAI : public Scripted_NoMovementAI
        {
            npc_dragon_soul_twilight_portalAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature)
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

            EventMap events;

            void Reset()
            {
                events.Reset();
            }

            void EnterCombat(Unit* /*who*/)
            {
                events.ScheduleEvent(EVENT_CHECK_PLAYERS, 5000);
            }

            void UpdateAI(uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (uint32 eventId = events.ExecuteEvent())
                {
                    if (!SelectTarget(SELECT_TARGET_RANDOM, 0, 40.0f))
                    {
                        events.Reset();
                        EnterEvadeMode();
                    }
                    else
                        events.ScheduleEvent(EVENT_CHECK_PLAYERS, 5000);
                }
            }
        };
};

class npc_dragon_soul_crimson_globule : public CreatureScript
{
    public:
        npc_dragon_soul_crimson_globule() : CreatureScript("npc_dragon_soul_crimson_globule") { }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return GetInstanceAI<npc_dragon_soul_crimson_globuleAI>(pCreature);
        }

        struct npc_dragon_soul_crimson_globuleAI : public ScriptedAI
        {
            npc_dragon_soul_crimson_globuleAI(Creature* pCreature) : ScriptedAI(pCreature)
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
                events.Reset();
            }

            void EnterCombat(Unit* /*who*/)
            {
                events.ScheduleEvent(EVENT_SEARING_BLOOD, urand(7000, 14000));
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
                    me->CastCustomSpell(SPELL_SEARING_BLOOD, SPELLVALUE_MAX_TARGETS, RAID_MODE(3, 8), me); 
                    events.ScheduleEvent(EVENT_SEARING_BLOOD, (me->HasAura(SPELL_GLOWING_BLOOD_OF_SHUMA_2) ? 7000 : 14000));
                }
                
                DoMeleeAttackIfReady();
            }
        private:
            EventMap events;
        };
};

class npc_dragon_soul_acidic_globule : public CreatureScript
{
    public:
        npc_dragon_soul_acidic_globule() : CreatureScript("npc_dragon_soul_acidic_globule") { }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return GetInstanceAI<npc_dragon_soul_acidic_globuleAI>(pCreature);
        }

        struct npc_dragon_soul_acidic_globuleAI : public ScriptedAI
        {
            npc_dragon_soul_acidic_globuleAI(Creature* pCreature) : ScriptedAI(pCreature)
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
                events.Reset();
            }

            void EnterCombat(Unit* /*who*/)
            {
                events.ScheduleEvent(EVENT_DIGESTIVE_ACID, urand(7000, 14000));
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
                    DoCastAOE(SPELL_DIGESTIVE_ACID);
                    events.ScheduleEvent(EVENT_DIGESTIVE_ACID, (me->HasAura(SPELL_GLOWING_BLOOD_OF_SHUMA_2) ? 7000 : 14000));
                }
                
                DoMeleeAttackIfReady();
            }
        private:
            EventMap events;
        };
};

class npc_dragon_soul_dark_globule : public CreatureScript
{
    public:
        npc_dragon_soul_dark_globule() : CreatureScript("npc_dragon_soul_dark_globule") { }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return GetInstanceAI<npc_dragon_soul_dark_globuleAI>(pCreature);
        }

        struct npc_dragon_soul_dark_globuleAI : public ScriptedAI
        {
            npc_dragon_soul_dark_globuleAI(Creature* pCreature) : ScriptedAI(pCreature)
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
                events.Reset();
            }

            void EnterCombat(Unit* /*who*/)
            {
                events.ScheduleEvent(EVENT_PSYCHIC_SLICE, urand(7000, 14000));
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
                    DoCastVictim(SPELL_PSYCHIC_SLICE);
                    events.ScheduleEvent(EVENT_PSYCHIC_SLICE, (me->HasAura(SPELL_GLOWING_BLOOD_OF_SHUMA_2) ? 7000 : 14000));
                }
                
                DoMeleeAttackIfReady();
            }
        private:
            EventMap events;
        };
};

class npc_dragon_soul_shadowed_globule : public CreatureScript
{
    public:
        npc_dragon_soul_shadowed_globule() : CreatureScript("npc_dragon_soul_shadowed_globule") { }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return GetInstanceAI<npc_dragon_soul_shadowed_globuleAI>(pCreature);
        }

        struct npc_dragon_soul_shadowed_globuleAI : public ScriptedAI
        {
            npc_dragon_soul_shadowed_globuleAI(Creature* pCreature) : ScriptedAI(pCreature)
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
                events.Reset();
            }

            void EnterCombat(Unit* /*who*/)
            {
                events.ScheduleEvent(EVENT_DEEP_CORRUPTION, urand(12000, 24000));
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
                    if (Unit* pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                        DoCast(pTarget, SPELL_DEEP_CORRUPTION);
                    events.ScheduleEvent(EVENT_DEEP_CORRUPTION, (me->HasAura(SPELL_GLOWING_BLOOD_OF_SHUMA_2) ? 12000 : 24000));
                }
                
                DoMeleeAttackIfReady();
            }
        private:
            EventMap events;
        };
};

class npc_dragon_soul_cobalt_globule : public CreatureScript
{
    public:
        npc_dragon_soul_cobalt_globule() : CreatureScript("npc_dragon_soul_cobalt_globule") { }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return GetInstanceAI<npc_dragon_soul_cobalt_globuleAI>(pCreature);
        }

        struct npc_dragon_soul_cobalt_globuleAI : public ScriptedAI
        {
            npc_dragon_soul_cobalt_globuleAI(Creature* pCreature) : ScriptedAI(pCreature)
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
                events.Reset();
            }

            void EnterCombat(Unit* /*who*/)
            {
                events.ScheduleEvent(EVENT_MANA_VOID, 3000);
            }

            void JustDied(Unit* /*killer*/)
            {
                DoCastAOE(SPELL_MANA_DIFFUSION);
            }

            void UpdateAI(uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (uint32 eventId = events.ExecuteEvent())
                    DoCast(me, SPELL_MANA_VOID);
                
                DoMeleeAttackIfReady();
            }
        private:
            EventMap events;
        };
};

class npc_dragon_soul_flail_of_gorath : public CreatureScript
{
    public:
        npc_dragon_soul_flail_of_gorath() : CreatureScript("npc_dragon_soul_flail_of_gorath") { }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return GetInstanceAI<npc_dragon_soul_flail_of_gorathAI>(pCreature);
        }

        struct npc_dragon_soul_flail_of_gorathAI : public Scripted_NoMovementAI
        {
            npc_dragon_soul_flail_of_gorathAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature)
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
                events.Reset();
            }

            void EnterCombat(Unit* /*who*/)
            {
                events.ScheduleEvent(EVENT_SLUDGE_SPEW, urand(2000, 4000));
                events.ScheduleEvent(EVENT_TENTACLE_TOSS, 10000);
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
                        case EVENT_SLUDGE_SPEW:
                            if (Unit* pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                                DoCast(pTarget, SPELL_SLUDGE_SPEW);
                            events.ScheduleEvent(EVENT_SLUDGE_SPEW, urand(6000, 8000));
                            break;
                        case EVENT_WILD_FLAIL:
                            DoCastAOE(SPELL_WILD_FLAIL);
                            events.ScheduleEvent(EVENT_WILD_FLAIL, urand(7000, 10000));
                            break;
                        default:
                            break;
                    }
                }
            }

        private:
            EventMap events;
        };
};

class npc_dragon_soul_claw_of_gorath : public CreatureScript
{
    public:
        npc_dragon_soul_claw_of_gorath() : CreatureScript("npc_dragon_soul_claw_of_gorath") { }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return GetInstanceAI<npc_dragon_soul_claw_of_gorathAI>(pCreature);
        }

        struct npc_dragon_soul_claw_of_gorathAI : public Scripted_NoMovementAI
        {
            npc_dragon_soul_claw_of_gorathAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature)
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
                events.Reset();
            }

            void EnterCombat(Unit* /*who*/)
            {
                events.ScheduleEvent(EVENT_OOZE_SPIT, 5000);
                events.ScheduleEvent(EVENT_TENTACLE_TOSS, 10000);
            }

            /*void SpellHitTarget(Unit* victim, const SpellInfo* spellInfo)
            {
                if (spellInfo->Id == SPELL_TENTACLE_TOSS_AOE_1)
                    events.RescheduleEvent(EVENT_TENTACLE_TOSS, 20000);
            }*/

            void UpdateAI(uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_OOZE_SPIT:
                            if (!me->IsWithinMeleeRange(me->getVictim()))
                                if (Unit* pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                                    DoCast(pTarget, SPELL_OOZE_SPIT);
                            events.ScheduleEvent(EVENT_OOZE_SPIT, 4000);
                            break;
                        case EVENT_TENTACLE_TOSS:
                            if (Unit* pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                                me->getVictim()->CastSpell(pTarget, SPELL_TENTACLE_TOSS_JUMP, true);
                            events.ScheduleEvent(EVENT_TENTACLE_TOSS, 20000);
                            break;
                        default:
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }

        private:
            EventMap events;
        };
};

/*class npc_dragon_tentacle_toss_target : public CreatureScript
{
    public:
        npc_dragon_tentacle_toss_target() : CreatureScript("npc_dragon_soul_tentacle_toss_target") { }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return GetInstanceAI<npc_dragon_soul_tentacle_toss_targetAI>(pCreature);
        }

        struct npc_dragon_soul_tentacle_toss_targetAI : public Scripted_NoMovementAI
        {
            npc_dragon_soul_tentacle_toss_targetAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature)
            {
            }

            void IsSummonedBy(Unit* owner)
            {
                DoCastAOE(SPELL_TENTACLE_TOSS_AOE_1, true);
            }
        };
};*/

class npc_dragon_soul_eye_of_gorath : public CreatureScript
{
    public:
        npc_dragon_soul_eye_of_gorath() : CreatureScript("npc_dragon_soul_eye_of_gorath") { }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return GetInstanceAI<npc_dragon_soul_eye_of_gorathAI>(pCreature);
        }

        struct npc_dragon_soul_eye_of_gorathAI : public Scripted_NoMovementAI
        {
            npc_dragon_soul_eye_of_gorathAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature)
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
                events.Reset();
            }

            void EnterCombat(Unit* /*who*/)
            {
                events.ScheduleEvent(EVENT_SHADOW_GAZE, urand(3000, 5000));
            }

            void JustDied(Unit* /*killer*/)
            {
                me->DespawnOrUnsummon(5 * IN_MILLISECONDS);
            }

            void UpdateAI(uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (uint32 eventId = events.ExecuteEvent())
                {
                    if (Unit* pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                        DoCast(pTarget, SPELL_SHADOW_GAZE);
                    events.ScheduleEvent(EVENT_SHADOW_GAZE, urand(3000, 4000));
                }
            }

        private:
            EventMap events;
        };
};

class npc_dragon_soul_teleport : public CreatureScript
{
    public:
        npc_dragon_soul_teleport() : CreatureScript("npc_dragon_soul_teleport") { }

        bool OnGossipHello(Player* pPlayer, Creature* pCreature)
        {
            if (pPlayer->isInCombat())
                return true;

            if (InstanceScript* instance = pCreature->GetInstanceScript())
                if (instance->GetData(DATA_ULTRAXION_TRASH) == SPECIAL)
                    return true;

            if (InstanceScript* instance = pCreature->GetInstanceScript())
            {
                switch (pCreature->GetEntry())
                {
                    case NPC_TRAVEL_TO_WYRMREST_BASE:
                        if (instance->GetBossState(DATA_MORCHOK) == DONE)
                            if (pCreature->GetPositionZ() > 50.0f && pCreature->GetPositionZ() < 100.0f)
                                pPlayer->NearTeleportTo(portalsPos[0].GetPositionX(), portalsPos[0].GetPositionY(), portalsPos[0].GetPositionZ(), portalsPos[0].GetOrientation());
                            else if (pCreature->GetPositionZ() < 50.0f || pCreature->GetPositionZ() > 100.0f)
                                pPlayer->NearTeleportTo(portalsPos[10].GetPositionX(), portalsPos[10].GetPositionY(), portalsPos[10].GetPositionZ(), portalsPos[10].GetOrientation());
                        break;
                    case NPC_TRAVEL_TO_WYRMREST_TEMPLE:
                        if (instance->GetBossState(DATA_MORCHOK) == DONE)
                            if (pCreature->GetPositionZ() < -200.0f)
                                pPlayer->NearTeleportTo(portalsPos[1].GetPositionX(), portalsPos[1].GetPositionY(), portalsPos[1].GetPositionZ(), portalsPos[1].GetOrientation());
                            else
                                pPlayer->NearTeleportTo(portalsPos[2].GetPositionX(), portalsPos[2].GetPositionY(), portalsPos[2].GetPositionZ(), portalsPos[2].GetOrientation());
                        break;
                    case NPC_VALEERA:
                        if (instance->GetBossState(DATA_MORCHOK) == DONE)
                            pPlayer->NearTeleportTo(portalsPos[3].GetPositionX(), portalsPos[3].GetPositionY(), portalsPos[3].GetPositionZ(), portalsPos[3].GetOrientation());
                        break;
                    case NPC_EIENDORMI:
                        if (instance->GetBossState(DATA_MORCHOK) == DONE)
                            pPlayer->NearTeleportTo(portalsPos[4].GetPositionX(), portalsPos[4].GetPositionY(), portalsPos[4].GetPositionZ(), portalsPos[4].GetOrientation());
                        break;
                    case NPC_NETHESTRASZ:
                        if (instance->GetBossState(DATA_YORSAHJ) == DONE && instance->GetBossState(DATA_ZONOZZ) == DONE)
                            pPlayer->NearTeleportTo(portalsPos[5].GetPositionX(), portalsPos[5].GetPositionY(), portalsPos[5].GetPositionZ(), portalsPos[5].GetOrientation());
                        break;
                    case NPC_TRAVEL_TO_EYE_OF_ETERNITY:
                        if (instance->GetBossState(DATA_YORSAHJ) == DONE && instance->GetBossState(DATA_ZONOZZ) == DONE)
                            pPlayer->NearTeleportTo(portalsPos[6].GetPositionX(), portalsPos[6].GetPositionY(), portalsPos[6].GetPositionZ(), portalsPos[6].GetOrientation());
                        break;
                    case NPC_TRAVEL_TO_WYRMREST_SUMMIT:
                        pPlayer->NearTeleportTo(portalsPos[7].GetPositionX(), portalsPos[7].GetPositionY(), portalsPos[7].GetPositionZ(), portalsPos[7].GetOrientation());
                        break;
                    case NPC_TRAVEL_TO_DECK:
                        if (instance->GetBossState(DATA_BLACKHORN) == DONE && instance->GetBossState(DATA_ULTRAXION) == DONE)
                            pPlayer->NearTeleportTo(portalsPos[8].GetPositionX(), portalsPos[8].GetPositionY(), portalsPos[8].GetPositionZ(), portalsPos[8].GetOrientation());
                        break;
                    case NPC_TRAVEL_TO_MAELSTORM:
                        if (instance->GetBossState(DATA_BLACKHORN) == DONE && instance->GetBossState(DATA_ULTRAXION) == DONE)
                        {
                            pPlayer->NearTeleportTo(portalsPos[9].GetPositionX(), portalsPos[9].GetPositionY(), portalsPos[9].GetPositionZ(), portalsPos[9].GetOrientation());
                            if (instance->GetBossState(DATA_MADNESS) == DONE)
                                pPlayer->AddAura(SPELL_CALM_MAELSTROM_SKYBOX, pPlayer);
                        }
                        break;
                    default:
                        break;
                }

            }
            return true;
        }
};

class npc_dragon_soul_thrall : public CreatureScript
{
    public:
        npc_dragon_soul_thrall() : CreatureScript("npc_dragon_soul_thrall") { }

        bool OnGossipHello(Player* pPlayer, Creature* pCreature)
        {
            if (pPlayer->isInCombat())
                return true;

            if (InstanceScript* instance = pCreature->GetInstanceScript())
            {
                if (instance->GetBossState(DATA_HAGARA) == !DONE)
                    return true;

                if (instance->GetBossState(DATA_HAGARA) == DONE && instance->GetData(DATA_DRAGON_SOUL_EVENT) != DONE && instance->GetBossState(DATA_ULTRAXION) != DONE)
                {
                    pPlayer->ADD_GOSSIP_ITEM_DB(GOSSIP_MENU_ULTRAXION_START, 0, GOSSIP_MENU_ULTRAXION_START, GOSSIP_ACTION_INFO_DEF + 1);
                    pPlayer->SEND_GOSSIP_MENU(pPlayer->GetGossipTextId(pCreature), pCreature->GetGUID());
                    return true;
                }
                else if (instance->GetBossState(DATA_HAGARA) == DONE && instance->GetData(DATA_DRAGON_SOUL_EVENT) == DONE && instance->GetData(DATA_ULTRAXION_TRASH) != DONE && instance->GetBossState(DATA_ULTRAXION) != DONE)
                {
                    pPlayer->ADD_GOSSIP_ITEM_DB(GOSSIP_MENU_ULTRAXION_START, 1, GOSSIP_MENU_ULTRAXION_START, GOSSIP_ACTION_INFO_DEF + 2);
                    pPlayer->SEND_GOSSIP_MENU(pPlayer->GetGossipTextId(pCreature), pCreature->GetGUID());
                    return true;
                }
            }

            pPlayer->SEND_GOSSIP_MENU(pPlayer->GetGossipTextId(pCreature), pCreature->GetGUID());
            return true;
        }

        bool OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action)
        {
            InstanceScript* instance = creature->GetInstanceScript();

            if (sender == GOSSIP_MENU_ULTRAXION_START && action == GOSSIP_ACTION_INFO_DEF + 1)
            {
                player->CLOSE_GOSSIP_MENU();
                creature->AI()->DoAction(ACTION_AFTER_HAGARA);
                creature->RemoveFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
            }
            else if (sender == GOSSIP_MENU_ULTRAXION_START && action == GOSSIP_ACTION_INFO_DEF + 2)
            {
                player->CLOSE_GOSSIP_MENU();
                creature->RemoveFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                creature->AI()->DoAction(ACTION_SPAWN_DRAGONS);
            }
            return true;
        }

        struct npc_dragon_soul_thrallAI : public ScriptedAI
        {
            npc_dragon_soul_thrallAI(Creature* pCreature) : ScriptedAI(pCreature) 
            {
                instance = pCreature->GetInstanceScript();
            }

            void Reset()
            {
                events.Reset();
                if (InstanceScript* instance = me->GetInstanceScript())
                    if (instance->GetBossState(DATA_ULTRAXION) == DONE)
                        return;
                    else if (instance->GetData(DATA_ULTRAXION_TRASH) == DONE)
                        me->AI()->DoAction(ACTION_LOAD_TRASH);
                    else if (instance->GetData(DATA_DRAGON_SOUL_EVENT) == DONE)
                        me->AI()->DoAction(ACTION_LOAD_EVENT);

                if (InstanceScript* instance = me->GetInstanceScript())
                    if (instance->GetBossState(DATA_ULTRAXION) != DONE && instance->GetData(DATA_DRAGON_SOUL_EVENT) == DONE)
                        if (Creature* DragonSoul = me->FindNearestCreature(NPC_THE_DRAGON_SOUL, 300.0f))
                        {
                            DragonSoul->RemoveAurasDueToSpell(SPELL_DRAGON_SOUL_COSMETIC);
                            DragonSoul->CastSpell(DragonSoul, SPELL_DRAGON_SOUL_COSMETIC);
                        }
            }

            void DoAction(const int32 action)
            {
                switch (action)
                {
                    case ACTION_ULTRAXION_WIN:
                        events.ScheduleEvent(EVENT_SPAWN_SHIP, 1000);
                        break;
                    case ACTION_AFTER_HAGARA:
                        events.ScheduleEvent(EVENT_AFTER_HAGARA, 1000);
                        break;
                    case ACTION_SPAWN_DRAGONS:
                        events.ScheduleEvent(EVENT_SPAWN_DRAGONS, 1000);
                        break;
                    case ACTION_ALEXSTRASZA:
                        events.ScheduleEvent(EVENT_DRAGONS_INTRO_1, 16000);
                        break;
                    case ACTION_START_ULTRAXION:
                        events.ScheduleEvent(EVENT_ULTRAXION_NEAR, 12000);
                        break;
                    case ACTION_LOAD_EVENT:
                        events.ScheduleEvent(EVENT_LOAD_EVENT, 5000);
                        break;
                    case ACTION_LOAD_TRASH:
                        events.ScheduleEvent(EVENT_LOAD_TRASH_1, 5000);
                        break;
                    case ACTION_STOP_ASSAULTERS_SPAWN:
                        events.CancelEvent(EVENT_NEXT_ASSAULTER);
                        events.CancelEvent(EVENT_TRASH_WIPE_CHECK);
                        break;
                }
            }

            void UpdateAI(uint32 diff)
            {
                events.Update(diff);

                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_LOAD_EVENT:
                            if (Creature* Ysera = me->FindNearestCreature(NPC_YSERA_THE_AWAKENED, 300.0f))
                                Ysera->CastSpell(Ysera, SPELL_CHARGING_UP_DREAMS);
                            if (Creature* Nozdormu = me->FindNearestCreature(NPC_NOZDORMU_THE_TIMELESS_ONE, 300.0f))
                                Nozdormu->CastSpell(Nozdormu, SPELL_CHARGING_UP_TIME);
                            if (Creature* Kalecgos = me->FindNearestCreature(NPC_KALECGOS, 300.0f))
                                Kalecgos->CastSpell(Kalecgos, SPELL_CHARGING_UP_MAGIC);
                            if (Creature* Alexstrasza = me->FindNearestCreature(NPC_ALEXSTRASZA_THE_LIFE_BINDER, 300.0f))
                                Alexstrasza->CastSpell(Alexstrasza, SPELL_CHARGING_UP_LIFE);
                            if (Creature* DragonSoul = me->FindNearestCreature(NPC_THE_DRAGON_SOUL, 300.0f))
                            {
                                DragonSoul->RemoveAurasDueToSpell(SPELL_DRAGON_SOUL_COSMETIC);
                                DragonSoul->CastSpell(DragonSoul, SPELL_DRAGON_SOUL_COSMETIC);
                            }
                            me->CastSpell(me, SPELL_CHARGING_UP_EARTH);
                            me->SetFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                            break;
                        case EVENT_LOAD_TRASH_1:
                            me->CastSpell(me, SPELL_WARD_OF_EARTH);
                            if (Creature* Ysera = me->FindNearestCreature(NPC_YSERA_THE_AWAKENED, 300.0f))
                                Ysera->CastSpell(Ysera, SPELL_WARD_OF_DREAMS);
                            if (Creature* Alexstrasza = me->FindNearestCreature(NPC_ALEXSTRASZA_THE_LIFE_BINDER, 300.0f))
                                Alexstrasza->CastSpell(Alexstrasza, SPELL_WARD_OF_LIFE);
                            if (Creature* Nozdormu = me->FindNearestCreature(NPC_NOZDORMU_THE_TIMELESS_ONE, 300.0f))
                                Nozdormu->CastSpell(Nozdormu, SPELL_WARD_OF_TIME);
                            if (Creature* Kalecgos = me->FindNearestCreature(NPC_KALECGOS, 300.0f))
                                Kalecgos->CastSpell(Kalecgos, SPELL_WARD_OF_MAGIC);
                           events.ScheduleEvent(EVENT_LOAD_TRASH_2, 1000);
                           break;
                        case EVENT_LOAD_TRASH_2:
                            if (Creature* DragonSoul = me->FindNearestCreature(NPC_THE_DRAGON_SOUL, 300.0f))
                            {
                                DragonSoul->GetMotionMaster()->MovePoint(0, DragonSoul->GetPositionX(), DragonSoul->GetPositionY(), DragonSoul->GetPositionZ() + 7);
                                me->RemoveFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                                me->SetFacingToObject(DragonSoul);
                                me->GetMotionMaster()->MovePoint(0, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ() + 3);
                                me->SummonCreature(NPC_ULTRAXION, ultraxionPos[0]);
                                if (Creature* Ysera = me->FindNearestCreature(NPC_YSERA_THE_AWAKENED, 300.0f))
                                {
                                    Ysera->SetFacingToObject(DragonSoul);
                                    Ysera->GetMotionMaster()->MovePoint(0, Ysera->GetPositionX(), Ysera->GetPositionY(), Ysera->GetPositionZ() + 3);
                                }
                                if (Creature* Alexstrasza = me->FindNearestCreature(NPC_ALEXSTRASZA_THE_LIFE_BINDER, 300.0f))
                                {
                                    Alexstrasza->SetFacingToObject(DragonSoul);
                                    Alexstrasza->GetMotionMaster()->MovePoint(0, Alexstrasza->GetPositionX(), Alexstrasza->GetPositionY(), Alexstrasza->GetPositionZ() + 3);
                                }
                                if (Creature* Nozdormu = me->FindNearestCreature(NPC_NOZDORMU_THE_TIMELESS_ONE, 300.0f))
                                {
                                    Nozdormu->SetFacingToObject(DragonSoul);
                                    Nozdormu->GetMotionMaster()->MovePoint(0, Nozdormu->GetPositionX(), Nozdormu->GetPositionY(), Nozdormu->GetPositionZ() + 3);
                                }
                                if (Creature* Kalecgos = me->FindNearestCreature(NPC_KALECGOS, 300.0f))
                                {
                                    Kalecgos->SetFacingToObject(DragonSoul);
                                    Kalecgos->GetMotionMaster()->MovePoint(0, Kalecgos->GetPositionX(), Kalecgos->GetPositionY(), Kalecgos->GetPositionZ() + 3);
                                }
                            }
                            break;
                        case EVENT_TRASH_WIPE_CHECK:
                            {
                                Map::PlayerList const& players = instance->instance->GetPlayers();
                                bool wipe = true;
                                for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
                                {
                                    if (Player* player = itr->getSource())
                                    {
                                        if (player->IsInWorld() && player->GetMapId() == me->GetMapId() && player->isAlive())
                                        {
                                            wipe = false;
                                            break;
                                        }
                                    }
                                }
                                if (wipe)
                                {
                                    events.CancelEvent(EVENT_TRASH_WIPE_CHECK);
                                    instance->SetData(DATA_ULTRAXION_TRASH, FAIL);
                                    if (Creature* Ysera = me->FindNearestCreature(NPC_YSERA_THE_AWAKENED, 50.0f))
                                        Ysera->Respawn(true);
                                    if (Creature* Alexstrasza = me->FindNearestCreature(NPC_ALEXSTRASZA_THE_LIFE_BINDER, 50.0f))
                                        Alexstrasza->Respawn(true);
                                    if (Creature* Nozdormu = me->FindNearestCreature(NPC_NOZDORMU_THE_TIMELESS_ONE, 50.0f))
                                        Nozdormu->Respawn(true);
                                    if (Creature* Kalecgos = me->FindNearestCreature(NPC_KALECGOS, 50.0f))
                                        Kalecgos->Respawn(true);
                                    if (Creature* DragonSoul = me->FindNearestCreature(NPC_THE_DRAGON_SOUL, 50.0f))
                                        DragonSoul->Respawn(true);
                                    me->Respawn(true);
                                    return;
                                }
                                events.ScheduleEvent(EVENT_TRASH_WIPE_CHECK, 5000);
                            }
                            break;
                        case EVENT_AFTER_HAGARA:
                            if (Creature* Ysera = me->FindNearestCreature(NPC_YSERA_THE_AWAKENED, 50.0f))
                                Ysera->AI()->Talk(SAY_YSERA_START);
                            events.ScheduleEvent(EVENT_DRAGON_SOUL_1, 4000);
                            break;
                        case EVENT_DRAGON_SOUL_1:
                            if (Creature* Ysera = me->FindNearestCreature(NPC_YSERA_THE_AWAKENED, 50.0f))
                                Ysera->CastSpell(Ysera, SPELL_CHARGING_UP_DREAMS);
                            events.ScheduleEvent(EVENT_DRAGON_SOUL_2, 1000);
                            break;
                        case EVENT_DRAGON_SOUL_2:
                            if (Creature* Nozdormu = me->FindNearestCreature(NPC_NOZDORMU_THE_TIMELESS_ONE, 50.0f))
                                Nozdormu->AI()->Talk(SAY_NOZDORMU_EVENT);
                            events.ScheduleEvent(EVENT_DRAGON_SOUL_3, 2500);
                            break;
                        case EVENT_DRAGON_SOUL_3:
                            if (Creature* Nozdormu = me->FindNearestCreature(NPC_NOZDORMU_THE_TIMELESS_ONE, 50.0f))
                                Nozdormu->CastSpell(Nozdormu, SPELL_CHARGING_UP_TIME);
                            events.ScheduleEvent(EVENT_DRAGON_SOUL_4, 500);
                            break;
                        case EVENT_DRAGON_SOUL_4:
                            if (Creature* Kalecgos = me->FindNearestCreature(NPC_KALECGOS, 50.0f))
                                Kalecgos->AI()->Talk(SAY_KALECGOS_EVENT);
                            events.ScheduleEvent(EVENT_DRAGON_SOUL_5, 4000);
                            break;
                        case EVENT_DRAGON_SOUL_5:
                            if (Creature* Kalecgos = me->FindNearestCreature(NPC_KALECGOS, 50.0f))
                                Kalecgos->CastSpell(Kalecgos, SPELL_CHARGING_UP_MAGIC);
                            events.ScheduleEvent(EVENT_DRAGON_SOUL_6, 2000);
                            break;
                        case EVENT_DRAGON_SOUL_6:
                            if (Creature* Alexstrasza = me->FindNearestCreature(NPC_ALEXSTRASZA_THE_LIFE_BINDER, 50.0f))
                                Alexstrasza->AI()->Talk(SAY_ALEXSTRASZA_EVENT_1);
                            events.ScheduleEvent(EVENT_DRAGON_SOUL_7, 9000);
                            break; 
                        case EVENT_DRAGON_SOUL_7:
                            if (Creature* Alexstrasza = me->FindNearestCreature(NPC_ALEXSTRASZA_THE_LIFE_BINDER, 50.0f))
                                Alexstrasza->CastSpell(Alexstrasza, SPELL_CHARGING_UP_LIFE);
                            events.ScheduleEvent(EVENT_DRAGON_SOUL_8, 2000);
                            break; 
                        case EVENT_DRAGON_SOUL_8:
                            Talk(SAY_THRALL_EVENT_1);
                            events.ScheduleEvent(EVENT_DRAGON_SOUL_9, 6000);
                            break; 
                        case EVENT_DRAGON_SOUL_9:
                            Talk(SAY_THRALL_EVENT_2);
                            events.ScheduleEvent(EVENT_DRAGON_SOUL_10, 10000);
                            break; 
                        case EVENT_DRAGON_SOUL_10:
                            Talk(SAY_THRALL_EVENT_3);
                            events.ScheduleEvent(EVENT_DRAGON_SOUL_11, 3000);
                            break;
                        case EVENT_DRAGON_SOUL_11:
                            me->CastSpell(me, SPELL_CHARGING_UP_EARTH);
                            me->SetFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                            instance->SetData(DATA_DRAGON_SOUL_EVENT, DONE);
                            break;
                        case EVENT_SPAWN_DRAGONS:
                            instance->SetData(DATA_ULTRAXION_TRASH, IN_PROGRESS);
                            if (Creature* Deathwing = instance->instance->GetCreature(instance->GetGuidData(DATA_DRAGON_SOUL_EVENT)))
                                Deathwing->AI()->DoAction(ACTION_DEATHWING_INTRO);
                            break;
                        case EVENT_DRAGONS_INTRO_1:
                            if (Creature* Alexstrasza = me->FindNearestCreature(NPC_ALEXSTRASZA_THE_LIFE_BINDER, 50.0f))
                                Alexstrasza->AI()->Talk(SAY_ALEXSTRASZA_EVENT_2);
                            events.ScheduleEvent(EVENT_DRAGONS_INTRO_2, 2000);
                            break;
                        case EVENT_DRAGONS_INTRO_2:
                            me->CastSpell(me, SPELL_WARD_OF_EARTH);
                            if (Creature* Ysera = me->FindNearestCreature(NPC_YSERA_THE_AWAKENED, 50.0f))
                                Ysera->CastSpell(Ysera, SPELL_WARD_OF_DREAMS);
                            if (Creature* Alexstrasza = me->FindNearestCreature(NPC_ALEXSTRASZA_THE_LIFE_BINDER, 50.0f))
                                Alexstrasza->CastSpell(Alexstrasza, SPELL_WARD_OF_LIFE);
                            if (Creature* Nozdormu = me->FindNearestCreature(NPC_NOZDORMU_THE_TIMELESS_ONE, 50.0f))
                                Nozdormu->CastSpell(Nozdormu, SPELL_WARD_OF_TIME);
                            if (Creature* Kalecgos = me->FindNearestCreature(NPC_KALECGOS, 50.0f))
                                Kalecgos->CastSpell(Kalecgos, SPELL_WARD_OF_MAGIC);
                           events.ScheduleEvent(EVENT_DRAGONS_INTRO_3, 1000);
                            break;
                         case EVENT_DRAGONS_INTRO_3:
                            if (Creature* DragonSoul = me->FindNearestCreature(NPC_THE_DRAGON_SOUL, 50.0f))
                            {
                                DragonSoul->GetMotionMaster()->MovePoint(0, DragonSoul->GetPositionX(), DragonSoul->GetPositionY(), DragonSoul->GetPositionZ() + 7);
                                me->SetFacingToObject(DragonSoul);
                                me->GetMotionMaster()->MovePoint(0, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ() + 3);
                                if (Creature* Ysera = me->FindNearestCreature(NPC_YSERA_THE_AWAKENED, 50.0f))
                                {
                                    Ysera->SetFacingToObject(DragonSoul);
                                    Ysera->GetMotionMaster()->MovePoint(0, Ysera->GetPositionX(), Ysera->GetPositionY(), Ysera->GetPositionZ() + 3);
                                }
                                if (Creature* Alexstrasza = me->FindNearestCreature(NPC_ALEXSTRASZA_THE_LIFE_BINDER, 50.0f))
                                {
                                    Alexstrasza->SetFacingToObject(DragonSoul);
                                    Alexstrasza->GetMotionMaster()->MovePoint(0, Alexstrasza->GetPositionX(), Alexstrasza->GetPositionY(), Alexstrasza->GetPositionZ() + 3);
                                }
                                if (Creature* Nozdormu = me->FindNearestCreature(NPC_NOZDORMU_THE_TIMELESS_ONE, 50.0f))
                                {
                                    Nozdormu->SetFacingToObject(DragonSoul);
                                    Nozdormu->GetMotionMaster()->MovePoint(0, Nozdormu->GetPositionX(), Nozdormu->GetPositionY(), Nozdormu->GetPositionZ() + 3);
                                }
                                if (Creature* Kalecgos = me->FindNearestCreature(NPC_KALECGOS, 50.0f))
                                {
                                    Kalecgos->SetFacingToObject(DragonSoul);
                                    Kalecgos->GetMotionMaster()->MovePoint(0, Kalecgos->GetPositionX(), Kalecgos->GetPositionY(), Kalecgos->GetPositionZ() + 3);
                                }
                                instance->SetData(DATA_ULTRAXION_TRASH, SPECIAL);
                                events.ScheduleEvent(EVENT_NEXT_ASSAULTER, 5000);
                                events.ScheduleEvent(EVENT_TRASH_WIPE_CHECK, 5000);
                            }
                            break;
                        case EVENT_NEXT_ASSAULTER:
                            instance->SetData(DATA_NEXT_ASSAULTER, 0);
                            events.ScheduleEvent(EVENT_NEXT_ASSAULTER, instance->GetData(DATA_NEXT_ASSAULTER));
                            break;
                        case EVENT_ULTRAXION_NEAR:
                            if (Creature* Ysera = me->FindNearestCreature(NPC_YSERA_THE_AWAKENED, 50.0f))
                                Ysera->AI()->Talk(SAY_YSERA_EVENT);
                            events.ScheduleEvent(EVENT_SPAWN_ULTRAXION, 5000);
                            break;
                        case EVENT_SPAWN_ULTRAXION:
                            me->SummonCreature(NPC_ULTRAXION, ultraxionPos[0]);
                            break;
                        case EVENT_SPAWN_SHIP:
                            if (GameObject* pShip = ObjectAccessor::GetGameObject(*me, instance->GetGuidData(DATA_HORDE_SHIP)))
                            {
                                pShip->SetFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_DESTROYED);
                                pShip->UpdateObjectVisibility();
                            }
                            if (GameObject* pShip = ObjectAccessor::GetGameObject(*me, instance->GetGuidData(DATA_ALLIANCE_SHIP_FIRST)))
                            {
                                pShip->SetFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_DESTROYED);
                                pShip->UpdateObjectVisibility();
                            }
                            if (GameObject* pShip = ObjectAccessor::GetGameObject(*me, instance->GetGuidData(DATA_ALLIANCE_SHIP)))
                            {
                                pShip->RemoveFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_DESTROYED);
                                pShip->UpdateObjectVisibility();
                            }
                            events.ScheduleEvent(EVENT_SPAWN_NPC, 1500);
                            break;
                        case EVENT_SPAWN_NPC:
                            me->SummonCreature(NPC_SKY_CAPTAIN_SWAYZE, customPos[1], TEMPSUMMON_MANUAL_DESPAWN, 0);
                            me->SummonCreature(NPC_KAANU_REEVS, customPos[2], TEMPSUMMON_MANUAL_DESPAWN, 0);
                            if (Creature* pSwayze = ObjectAccessor::GetCreature(*me, instance->GetGuidData(DATA_SWAYZE)))
                                pSwayze->AI()->Talk(9);
                            events.ScheduleEvent(EVENT_TALK_ULTRAXION_WIN_1, 10000);
                            break;
                        case EVENT_TALK_ULTRAXION_WIN_1:
                            if (Creature* pAlexstrasza = me->FindNearestCreature(NPC_ALEXSTRASZA_THE_LIFE_BINDER, 50.0f))
                                pAlexstrasza->AI()->Talk(SAY_ALEXSTRASZA_WIN);
                            events.ScheduleEvent(EVENT_TALK_ULTRAXION_WIN_2, 15000);
                            break;
                        case EVENT_TALK_ULTRAXION_WIN_2:
                            Talk(SAY_THRALL_WIN);
                            break;
                        default:
                            break;
                    }
                }
            }

            private:
                InstanceScript* instance;
                EventMap events;
        };

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return GetInstanceAI<npc_dragon_soul_thrallAI>(pCreature);
        }
};

class npc_dragon_soul_deathwing_event : public CreatureScript
{
    public:
        npc_dragon_soul_deathwing_event() : CreatureScript("npc_dragon_soul_deathwing_event") { }

        struct npc_dragon_soul_deathwing_eventAI : public ScriptedAI
        {
            npc_dragon_soul_deathwing_eventAI(Creature* pCreature) : ScriptedAI(pCreature) 
            {
                me->SetWalk(false);
                me->SetSpeed(MOVE_FLIGHT, 5, true);
                me->SetVisible(true);
            }

            void Reset()
            {
                events.Reset();
                events.ScheduleEvent(EVENT_DEATHWING_OOC_VISUAL, 5000);
            }

            void DoAction(const int32 action)
            {
                switch (action)
                {
                    case ACTION_DEATHWING_INTRO:
                        events.Reset();
                        me->GetMotionMaster()->Clear();
                        me->GetMotionMaster()->MovePoint(POINT_EVENT_1, DeathwingEventPos);
                        break;
                    case ACTION_DEATHWING_RESET:
                        Reset();
                        me->SetVisible(true);
                        break;
                }
            }

            void MovementInform(uint32 type, uint32 point)
            {
                if (type != POINT_MOTION_TYPE)
                    return;

                switch (point)
                {
                    case POINT_OOC_1:
                        me->SetFacingTo(me->GetAngle(&customPos[5]));
                        events.ScheduleEvent(EVENT_DEATHWING_OOC_CAST_1, 5000);
                        break;
                    case POINT_OOC_2:
                        me->SetFacingTo(me->GetAngle(&customPos[5]));
                        events.ScheduleEvent(EVENT_DEATHWING_OOC_CAST_2, 5000);
                        break;
                    case POINT_OOC_3:
                        me->SetFacingTo(me->GetAngle(&customPos[5]));
                        events.ScheduleEvent(EVENT_DEATHWING_OOC_CAST_3, 5000);
                        break;
                    case POINT_EVENT_1:
                        if (InstanceScript* instance = me->GetInstanceScript())
                            if (instance->GetData(DATA_ULTRAXION_TRASH)==IN_PROGRESS)
                                events.ScheduleEvent(EVENT_DEATHWING_INTRO_1, 1000);
                            else if (instance->GetData(DATA_ULTRAXION_TRASH)==DONE)
                                events.ScheduleEvent(EVENT_DEATHWING_INTRO_3, 1000);
                        break;
                    case POINT_LAST:
                        me->SetVisible(false);
                        break;
                    default:
                        break;
                }
            }

            void UpdateAI(uint32 diff)
            {
                events.Update(diff);

                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_DEATHWING_OOC_VISUAL:
                            switch (urand(1, 3))
                            {
                                case POINT_OOC_1:
                                    me->GetMotionMaster()->MovePoint(POINT_OOC_1, DeathwingOOCVisual[POINT_OOC_1 - 1]);
                                    break;
                                case POINT_OOC_2:
                                    me->GetMotionMaster()->MovePoint(POINT_OOC_2, DeathwingOOCVisual[POINT_OOC_2 - 1]);
                                    break;
                                case POINT_OOC_3:
                                    me->GetMotionMaster()->MovePoint(POINT_OOC_3, DeathwingOOCVisual[POINT_OOC_3 - 1]);
                                    break;
                            }
                            break;
                        case EVENT_DEATHWING_OOC_CAST_1:
                            DoCastAOE(SPELL_MOLTEN_METEOR);
                            events.ScheduleEvent(EVENT_DEATHWING_OOC_RESET, 5000);
                            break;
                        case EVENT_DEATHWING_OOC_CAST_2:
                            DoCastAOE(SPELL_MOLTEN_METEOR);
                            events.ScheduleEvent(EVENT_DEATHWING_OOC_RESET, 5000);
                            break;
                        case EVENT_DEATHWING_OOC_CAST_3:
                            DoCastAOE(SPELL_MOLTEN_METEOR);
                            events.ScheduleEvent(EVENT_DEATHWING_OOC_RESET, 5000);
                            break;
                        case EVENT_DEATHWING_OOC_RESET:
                            Reset();
                            break;
                        case EVENT_DEATHWING_INTRO_1:
                            Talk(SAY_DEATHWING_INTRO_1);
                            me->SetFacingTo(3.195251f);
                            events.ScheduleEvent(EVENT_DEATHWING_INTRO_2, 11000);
                            break;
                        case EVENT_DEATHWING_INTRO_2:
                            Talk(SAY_DEATHWING_INTRO_2);
                            if (Creature* Thrall = me->FindNearestCreature(NPC_THRALL_1, 150.0f))
                                Thrall->AI()->DoAction(ACTION_ALEXSTRASZA);
                            events.ScheduleEvent(EVENT_DEATHWING_OOC_RESET, 10000);
                            break;
                        case EVENT_DEATHWING_INTRO_3:
                            Talk(SAY_DEATHWING_INTRO_3);
                            me->SetFacingTo(3.195251f);
                            events.ScheduleEvent(EVENT_DEATHWING_INTRO_4, 14000);
                            break;
                        case EVENT_DEATHWING_INTRO_4:
                            Talk(SAY_DEATHWING_INTRO_4);
                            events.ScheduleEvent(EVENT_DEATHWING_INTRO_5, 13000);
                            break;
                        case EVENT_DEATHWING_INTRO_5:
                            me->AI()->Talk(SAY_DEATHWING_INTRO_5);
                            if (Creature* Thrall = me->FindNearestCreature(NPC_THRALL_1, 150.0f))
                                Thrall->AI()->DoAction(ACTION_START_ULTRAXION);
                            events.ScheduleEvent(EVENT_DEATHWING_DESPAWN, 10000);
                            break;
                        case EVENT_DEATHWING_DESPAWN:
                            me->GetMotionMaster()->MovePoint(POINT_LAST, DeathwingDespawnPos);
                            break;
                        default:
                            break;
                    }
                }
            }
        private:
            EventMap events;

        };

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return GetInstanceAI<npc_dragon_soul_deathwing_eventAI>(pCreature);
        }
};

class npc_dragon_soul_twilight_assaulter : public CreatureScript
{
    public:
        npc_dragon_soul_twilight_assaulter() : CreatureScript("npc_dragon_soul_twilight_assaulter") { }

        struct npc_dragon_soul_twilight_assaulterAI : public ScriptedAI
        {
            npc_dragon_soul_twilight_assaulterAI(Creature* pCreature) : ScriptedAI(pCreature)
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
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NOT_SELECTABLE);
                wasActivated = false;
                wasAssaulting = false;
                wasChanneling = false;
                horizontal = false;
                lane = 0;
                me->SetReactState(REACT_PASSIVE);
            }

            void Reset()
            {
                if (me->isDead())
                    return;

                events.Reset();
                me->SetCanFly(true);
                me->SetDisableGravity(true);

                instance = me->GetInstanceScript();
                if (!instance)
                {
                    me->DespawnOrUnsummon();
                    return;
                }
                accessor = CAST_AI(instance_dragon_soul_trash_accessor, me->GetInstanceScript());

                if (wasChanneling)
                    accessor->CleanTwilightAssaulterAssaultLane(horizontal, lane);

                if (wasAssaulting)
                    accessor->FreeTwilightAssaulterAssaultLane(horizontal, lane);

                wasAssaulting = false;
                wasChanneling = false;
                horizontal = false;
                lane = 0;

                events.ScheduleEvent(EVENT_ASSAULTER_VISUAL, urand(30000, 60000));
                me->CastSpell(me, SPELL_TEMPERAMENT);
            }

            void EnterCombat(Unit* who)
            {
                if (me->GetReactState() == REACT_PASSIVE)
                {
                    me->CombatStop(true);
                    me->getHostileRefManager().deleteReferences();
                    if (wasChanneling)
                        DoCastAOE(SPELL_TWLIGHT_FLAMES_CHANNEL);
                    return;
                }

                ScriptedAI::EnterCombat(who);

                if (wasChanneling)
                    accessor->CleanTwilightAssaulterAssaultLane(horizontal, lane);

                if (wasAssaulting)
                    accessor->FreeTwilightAssaulterAssaultLane(horizontal, lane);

                wasAssaulting = false;
                wasChanneling = false;

                events.CancelEvent(EVENT_ASSAULTER_VISUAL);
                events.ScheduleEvent(EVENT_ASSAULTER_BREATH, urand(20000, 30000));
            }

            void JustDied(Unit* /*killer*/)
            {
                me->GetMotionMaster()->Clear();
                me->GetMotionMaster()->MoveFall();
                instance->SetData(DATA_DRAGONS_COUNT, 0);
            }

            void SpellHit(Unit* caster, SpellInfo const* spell)
            {
                if (spell->HasEffect(SPELL_EFFECT_ATTACK_ME) || spell->HasAura(SPELL_AURA_MOD_TAUNT))
                {
                    me->SetReactState(REACT_AGGRESSIVE);
                    AttackStart(caster);
                }
            }

            void MovementInform(uint32 type, uint32 point)
            {
                if (type != POINT_MOTION_TYPE || point != POINT_ASSAULTER)
                    return;
                if (!wasAssaulting || wasChanneling)
                    return;
                
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
                me->GetMotionMaster()->MoveIdle();
                me->SetFacingTo(assaultPos.GetOrientation());
                wasChanneling = true;
                DoCastAOE(SPELL_TWLIGHT_FLAMES_CHANNEL);
            }

            void DoAction(int32 const action)
            {
                if (action == ACTION_START_ASSAULT)
                {
                    events.CancelEvent(EVENT_ASSAULTER_VISUAL);

                    const Position* posPtr;
                    float angle = customPos[5].GetAngle(me);
                    if (angle <= M_PI/4 || angle > M_PI*2 - M_PI/4)
                        posPtr = accessor->GetRandomTwilightAssaulterAssaultPosition(horizontal = false, false, lane, stalkerGUID); // North
                    else if (angle <= M_PI/2 + M_PI/4)
                        posPtr = accessor->GetRandomTwilightAssaulterAssaultPosition(horizontal = true, false, lane, stalkerGUID); // West
                    else if (angle <= M_PI + M_PI/4)
                        posPtr = accessor->GetRandomTwilightAssaulterAssaultPosition(horizontal = false, true, lane, stalkerGUID); // South
                    else
                        posPtr = accessor->GetRandomTwilightAssaulterAssaultPosition(horizontal = true, true, lane, stalkerGUID); // East

                    if (!posPtr)
                        return;
                    assaultPos = Position(*posPtr);
                    delete posPtr;

                    wasActivated = true;
                    wasAssaulting = true;
                    me->GetMotionMaster()->MoveIdle();
                    me->GetMotionMaster()->MovePoint(POINT_ASSAULTER, assaultPos);
                }
                else if (action == ACTION_STOP_ASSAULT)
                {
                    wasActivated = false;
                    if (wasAssaulting)
                    {
                        wasAssaulting = false;
                        me->GetMotionMaster()->MoveIdle();
                        if (wasChanneling)
                        {
                            wasChanneling = false;
                            me->InterruptNonMeleeSpells(true);
                        }
                    }

                    if (me->isInCombat())
                    {
                        me->CombatStop(true);
                        me->getHostileRefManager().deleteReferences();
                        EnterEvadeMode();
                    }
                }
            }

            ObjectGuid GetGUID(int32 type = 0)
            {
                return stalkerGUID;
            }

            uint32 GetData(uint32 type) const override
            {
                if (type == 1)
                    return wasActivated ? 1 : 0;
                return 0;
            }

            void UpdateAI(uint32 diff)
            {
                events.Update(diff);
                switch (events.ExecuteEvent())
                {
                case EVENT_ASSAULTER_VISUAL:
                    if (me->GetPhaseMask() == 1)
                        DoCastAOE(SPELL_TWILIGHT_FIREBALL);
                    events.ScheduleEvent(EVENT_ASSAULTER_VISUAL, urand(30000, 60000));
                    break;
                case EVENT_ASSAULTER_BREATH:
                    if (me->getVictim())
                        DoCastVictim(SPELL_TWILIGHT_BREATH);
                    events.ScheduleEvent(EVENT_ASSAULTER_BREATH, urand(20000, 30000));
                    break;
                default:
                    break;
                }

                DoMeleeAttackIfReady();
            }

        private:
            EventMap events;
            InstanceScript* instance;
            instance_dragon_soul_trash_accessor* accessor;
            bool wasActivated, wasAssaulting, wasChanneling;
            bool horizontal;
            uint8 lane;
            Position assaultPos;
            ObjectGuid stalkerGUID;
        };

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return GetInstanceAI<npc_dragon_soul_twilight_assaulterAI>(pCreature);
        }
};

class npc_dragon_soul_kalecgos_event : public CreatureScript
{
    public:
        npc_dragon_soul_kalecgos_event() : CreatureScript("npc_dragon_soul_kalecgos_event") { }

        struct npc_dragon_soul_kalecgos_eventAI : public ScriptedAI
        {
            npc_dragon_soul_kalecgos_eventAI(Creature* pCreature) : ScriptedAI(pCreature)
            {
                instance = me->GetInstanceScript();
                bIntroDone = false;
            }

            bool bIntroDone;

            void Reset()
            {
                if (me->GetPositionZ() < 200)
                    events.ScheduleEvent(EVENT_CHARGE_DRAGON_SOUL, 1000);
            }

            void MoveInLineOfSight(Unit* who)
            {
                if (bIntroDone)
                    return;

                if (instance->GetData(DATA_OPEN_PORTAL_TO_EYE) == DONE)
                    return;

                 if ((instance->GetBossState(DATA_YORSAHJ) != DONE) || (instance->GetBossState(DATA_ZONOZZ) != DONE))
                    return;

                if (who->GetTypeId() != TYPEID_PLAYER)
                    return;

                if (!me->IsWithinDistInMap(who, 20.0f, false))
                    return;

                events.ScheduleEvent(EVENT_OPEN_PORTAL_START, 15000);
                bIntroDone = true;
            }

            void UpdateAI(uint32 diff)
            {
                events.Update(diff);
                switch (events.ExecuteEvent())
                {
                    case EVENT_OPEN_PORTAL_START:
                        if (Creature* Thrall = me->FindNearestCreature(NPC_THRALL_1, 50.0f))
                            Thrall->AI()->Talk(SAY_THRALL_PORTAL_1); 
                        events.ScheduleEvent(EVENT_OPEN_PORTAL_1, 13000);
                        break;
                    case EVENT_OPEN_PORTAL_1:
                        if (Creature* Alexstrasza = me->FindNearestCreature(NPC_ALEXSTRASZA_THE_LIFE_BINDER, 50.0f))
                            Alexstrasza->AI()->Talk(SAY_ALEXSTRASZA_PORTAL);
                        events.ScheduleEvent(EVENT_OPEN_PORTAL_2, 6000);
                        break;
                    case EVENT_OPEN_PORTAL_2:
                        Talk(SAY_KAKECGOS_PORTAL_1);
                        events.ScheduleEvent(EVENT_OPEN_PORTAL_3, 12000);
                        break;
                    case EVENT_OPEN_PORTAL_3:
                        Talk(SAY_KAKECGOS_PORTAL_2);
                        events.ScheduleEvent(EVENT_OPEN_PORTAL_4, 13000);
                        break;
                    case EVENT_OPEN_PORTAL_4:
                        if (Creature* Thrall = me->FindNearestCreature(NPC_THRALL_1, 50.0f))
                            Thrall->AI()->Talk(SAY_THRALL_PORTAL_2); 
                        events.ScheduleEvent(EVENT_OPEN_PORTAL_5, 12000);
                        break;
                    case EVENT_OPEN_PORTAL_5:
                        if (Creature* Ysera = me->FindNearestCreature(NPC_YSERA_THE_AWAKENED, 50.0f))
                            Ysera->AI()->Talk(SAY_YSERA_PORTAL); 
                        events.ScheduleEvent(EVENT_OPEN_PORTAL_6, 12000);
                        break;
                    case EVENT_OPEN_PORTAL_6:
                        if (Creature* Thrall = me->FindNearestCreature(NPC_THRALL_1, 50.0f))
                            Thrall->AI()->Talk(SAY_THRALL_PORTAL_3); 
                        events.ScheduleEvent(EVENT_OPEN_PORTAL_7, 6000);
                        break;
                    case EVENT_OPEN_PORTAL_7:
                        Talk(SAY_KAKECGOS_PORTAL_3);
                        if (Creature* Portal = me->FindNearestCreature(NPC_TRAVEL_TO_EYE_OF_ETERNITY, 50.0f))
                            Portal->SetVisible(true);
                        events.ScheduleEvent(EVENT_OPEN_PORTAL_8, 5000);
                        break;
                    case EVENT_OPEN_PORTAL_8:
                        instance->SetData(DATA_OPEN_PORTAL_TO_EYE, DONE);
                        break;
                    case EVENT_CHARGE_DRAGON_SOUL:
                        me->CastSpell(me, SPELL_CHARGE_DRAGON_SOUL_MAGIC);
                        if (Creature* Thrall = me->FindNearestCreature(NPC_THRALL_ON_SHIP, 50.0f))
                            Thrall->CastSpell(Thrall, SPELL_CHARGE_DRAGON_SOUL_EARTH);
                        if (Creature* Ysera = me->FindNearestCreature(NPC_YSERA_THE_AWAKENED, 50.0f))
                            Ysera->CastSpell(Ysera, SPELL_CHARGE_DRAGON_SOUL_DREAMS);
                        if (Creature* Alexstrasza = me->FindNearestCreature(NPC_ALEXSTRASZA_THE_LIFE_BINDER, 50.0f))
                            Alexstrasza->CastSpell(Alexstrasza, SPELL_CHARGE_DRAGON_SOUL_LIFE);
                        if (Creature* Nozdormu = me->FindNearestCreature(NPC_NOZDORMU_THE_TIMELESS_ONE, 50.0f))
                            Nozdormu->CastSpell(Nozdormu, SPELL_CHARGE_DRAGON_SOUL_TIME);
                        break;
                    default:
                        break;
                }
            }

        private:
            EventMap events;
            InstanceScript* instance;
        };

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return GetInstanceAI<npc_dragon_soul_kalecgos_eventAI>(pCreature);
        }
};



class spell_dragon_soul_trigger_spell_from_aoe : public SpellScriptLoader
{
    public:
        spell_dragon_soul_trigger_spell_from_aoe(char const* scriptName, uint32 triggerId) : SpellScriptLoader(scriptName), _triggerId(triggerId){ }

        class spell_dragon_soul_trigger_spell_from_aoe_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dragon_soul_trigger_spell_from_aoe_SpellScript);
            
        public:
            spell_dragon_soul_trigger_spell_from_aoe_SpellScript(uint32 triggerId) : SpellScript(), _triggerId(triggerId) { }

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                if (!GetCaster())
                    return;

                if (targets.size() <= 1)
                    return;

                std::list<WorldObject*> tempList;

                for (std::list<WorldObject*>::const_iterator itr = targets.begin(); itr != targets.end(); ++itr)
                    if (GetCaster()->GetDistance((*itr)) >= 15.0f)
                        tempList.push_back((*itr));

                if (!tempList.empty())
                {
                    targets.clear();
                    targets.push_back(Trinity::Containers::SelectRandomContainerElement(tempList));
                }
                else
                    Trinity::Containers::RandomResizeList(targets, 1);
            }

            void HandleDummy()
            {
                if (!GetCaster() || !GetHitUnit())
                    return;

                GetCaster()->CastSpell(GetHitUnit(), _triggerId, true);
            }

            void Register()
            {
                if (m_scriptSpellId == SPELL_BOULDER_SMASH_AOE)
                {
                    OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_dragon_soul_trigger_spell_from_aoe_SpellScript::FilterTargets, EFFECT_0,TARGET_UNIT_SRC_AREA_ENEMY);
                }
                AfterHit += SpellHitFn(spell_dragon_soul_trigger_spell_from_aoe_SpellScript::HandleDummy);
            }

        private:
            uint32 _triggerId;
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dragon_soul_trigger_spell_from_aoe_SpellScript(_triggerId);
        }

    private:
        uint32 _triggerId;
};

class spell_dragon_soul_shadowed_globule_deep_corruption : public SpellScriptLoader
{
    public:
        spell_dragon_soul_shadowed_globule_deep_corruption() : SpellScriptLoader("spell_dragon_soul_shadowed_globule_deep_corruption") { }

        class spell_dragon_soul_shadowed_globule_deep_corruption_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dragon_soul_shadowed_globule_deep_corruption_AuraScript);

            void HandlePeriodicTick(AuraEffect const* aurEff)
            {
                if (!GetCaster())
                    return;

                if (Aura* aur = GetAura())
                {
                    if (aur->GetStackAmount() >= 5)
                    {
                        GetCaster()->CastSpell(GetCaster(), SPELL_DEEP_CORRUPTION_DMG, true);
                        aur->Remove();
                    }
                }
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_dragon_soul_shadowed_globule_deep_corruption_AuraScript::HandlePeriodicTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dragon_soul_shadowed_globule_deep_corruption_AuraScript();
        }
};

class spell_dragon_soul_cobalt_globule_mana_void : public SpellScriptLoader
{
    public:
        spell_dragon_soul_cobalt_globule_mana_void() : SpellScriptLoader("spell_dragon_soul_cobalt_globule_mana_void") { }

        class spell_dragon_soul_cobalt_globule_mana_void_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dragon_soul_cobalt_globule_mana_void_SpellScript);

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                if (!GetCaster())
                    return;

                if (targets.empty())
                    return;

                targets.remove_if(ManaCheck());
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_dragon_soul_cobalt_globule_mana_void_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_dragon_soul_cobalt_globule_mana_void_SpellScript::FilterTargets, EFFECT_1, TARGET_UNIT_SRC_AREA_ENEMY);
            }

        private:
            class ManaCheck
            {
                public:
                    ManaCheck() {}
            
                    bool operator()(WorldObject* unit)
                    {
                        if (unit->GetTypeId() != TYPEID_PLAYER)
                            return true;
                        return (unit->ToPlayer()->getPowerType() != POWER_MANA);
                    }
            };

        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dragon_soul_cobalt_globule_mana_void_SpellScript();
        }
};

class spell_dragon_soul_twilight_flames_aoe : public SpellScriptLoader
{
    public:
        spell_dragon_soul_twilight_flames_aoe() : SpellScriptLoader("spell_dragon_soul_twilight_flames_aoe") { }

        class spell_dragon_soul_twilight_flames_aoe_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dragon_soul_twilight_flames_aoe_SpellScript);

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                Unit* caster = GetCaster();
                if (!caster)
                    return;

                targets.clear();
                if (caster->GetEntry() == NPC_TWILIGHT_ASSAULTER_STALKER)
                {
                    if (instance_dragon_soul_trash_accessor* accessor = CAST_AI(instance_dragon_soul_trash_accessor, caster->GetInstanceScript()))
                        if (Creature* nextStalker = accessor->GetNextTwilightAssaulterStalker(caster->ToCreature()))
                            targets.push_back(nextStalker);
                }
                else if (caster->ToCreature())
                {
                    if (InstanceScript* instance = caster->GetInstanceScript())
                        if (Creature* firstStalker = instance->instance->GetCreature(caster->ToCreature()->AI()->GetGUID()))
                            targets.push_back(firstStalker);
                }
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_dragon_soul_twilight_flames_aoe_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_dragon_soul_twilight_flames_aoe_SpellScript::FilterTargets, EFFECT_1, TARGET_UNIT_SRC_AREA_ENTRY);
            }
        };

        class spell_dragon_soul_twilight_flames_aoe_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dragon_soul_twilight_flames_aoe_AuraScript);

            void ApplyEffect(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (GetTarget())
                    GetTarget()->AddAura(SPELL_TWLIGHT_FLAMES_CHANNEL, GetTarget());
            }
            void RemoveEffect(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (GetTarget())
                    GetTarget()->RemoveAurasDueToSpell(SPELL_TWLIGHT_FLAMES_CHANNEL);
            }

            void Register()
            {
                OnEffectApply += AuraEffectApplyFn(spell_dragon_soul_twilight_flames_aoe_AuraScript::ApplyEffect, EFFECT_1, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
                OnEffectRemove += AuraEffectRemoveFn(spell_dragon_soul_twilight_flames_aoe_AuraScript::RemoveEffect, EFFECT_1, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dragon_soul_twilight_flames_aoe_SpellScript();
        }
        AuraScript* GetAuraScript() const
        {
            return new spell_dragon_soul_twilight_flames_aoe_AuraScript();
        }
};

void AddSC_dragon_soul()
{
    new npc_dragon_soul_ancient_water_lord();
    new npc_dragon_soul_earthen_destroyer();
    new npc_dragon_soul_earthen_soldier();
    new npc_dragon_soul_twilight_siege_captain();
    new npc_dragon_soul_twilight_portal();
    new npc_dragon_soul_crimson_globule();
    new npc_dragon_soul_acidic_globule();
    new npc_dragon_soul_dark_globule();
    new npc_dragon_soul_shadowed_globule();
    new npc_dragon_soul_cobalt_globule();
    new npc_dragon_soul_flail_of_gorath();
    new npc_dragon_soul_claw_of_gorath();
    new npc_dragon_soul_eye_of_gorath();
    new npc_dragon_soul_teleport();
    new npc_dragon_soul_thrall();
    new npc_dragon_soul_deathwing_event();
    new npc_dragon_soul_twilight_assaulter();
    new npc_dragon_soul_kalecgos_event();
    new spell_dragon_soul_trigger_spell_from_aoe("spell_dragon_soul_ancient_water_lord_flood", SPELL_FLOOD);
    new spell_dragon_soul_trigger_spell_from_aoe("spell_dragon_soul_earthen_destroyer_boulder_smash", SPELL_BOULDER_SMASH);
    new spell_dragon_soul_shadowed_globule_deep_corruption();
    new spell_dragon_soul_cobalt_globule_mana_void();
    new spell_dragon_soul_twilight_flames_aoe();
    //new spell_dragon_soul_trigger_spell_from_aoe("spell_dragon_soul_claw_of_gorath_tentacle_toss_aoe_1", SPELL_TENTACLE_TOSS_SCRIPT_1);
    //new spell_dragon_soul_trigger_spell_from_aoe("spell_dragon_soul_claw_of_gorath_tentacle_toss_aoe_2", SPELL_TENTACLE_TOSS_SUMMON);
}