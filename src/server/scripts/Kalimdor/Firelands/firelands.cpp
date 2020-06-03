#include "MoveSplineInit.h"
#include "Cell.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "firelands.h"
#include "ObjectVisitors.hpp"
#include "QuestData.h"

enum Spells
{
    // Ancient Core Hound
    SPELL_DINNER_TIME                           = 99693,
    SPELL_DINNER_TIME_VEHICLE                   = 99694,
    SPELL_FLAME_BREATH                          = 99736,
    SPELL_TERRIFYING_ROAR                       = 99692,

    // Ancient Lava Dweller
    SPELL_LAVA_SHOWER                           = 97549,
    SPELL_LAVA_SHOWER_MISSILE                   = 97551,
    SPELL_LAVA_SPIT                             = 97306,

    // Fire Scorpion
    SPELL_FIERY_BLOOD                           = 99993,
    SPELL_SLIGHTLY_WARM_PINCERS                 = 99984,

    // Fire Turtle Hatchling 
    SPELL_SHELL_SPIN                            = 100263,

    // Fire Archon
    SPELL_FLAME_TORRENT                         = 100795,
    SPELL_FIERY_TORMENT                         = 100797,
    SPELL_FIERY_TORMENT_DMG                     = 100802,

    // Molten Lord
    SPELL_FLAME_STOMP                           = 99530,
    SPELL_MELT_ARMOR                            = 99532,
    SPELL_SUMMON_LAVA_JETS                      = 99555,
    SPELL_SUMMON_LAVA_JET                       = 99538,

    // Molten Flamefather
    SPELL_EARTHQUAKE                            = 100724,
    SPELL_MAGMA_CONDUIT                         = 100728,

    // Magma Conduit
    SPELL_VOLCANO_SMOKE                         = 97699,
    SPELL_VOLCANO_BASE                          = 98250,
    SPELL_SUMMON_MAGMAKIN                       = 100746,
    SPELL_SUMMON_MAGMAKIN_DMG                   = 100748,

    // Magmakin
    SPELL_ERUPTION                              = 100755,

    // Harbinger of Flame
    SPELL_FIRE_IT_UP                            = 100093,
    SPELL_FIEROBLAST_TRASH                      = 100094,
    SPELL_FIEROCLAST_BARRAGE                    = 100095,
    SPELL_FIRE_CHANNELING                       = 100109,

    // Blazing Monstrosity
    SPELL_RIDE_MONSTROSITY                      = 93970,
    SPELL_SHARE_HEALTH_LEFT                     = 101502,
    SPELL_SHARE_HEALTH_RIGHT                    = 101503,
    SPELL_SLEEP_ULTRA_HIGH_PRIORITY             = 99480,
    SPELL_GENERIC_DUMMY_CAST                    = 100088,
    SPELL_LEFT_SIDE_SMACK_L                     = 100076,
    SPELL_RIGHT_SIDE_SMACK_L                    = 100078,
    SPELL_HEAD_BONK_L                           = 100080,
    SPELL_TICKLE_L                              = 100082,
    SPELL_KNOCKBACK_RIGHT                       = 100084,
    SPELL_KNOCKBACK_LEFT                        = 100085,
    SPELL_KNOCKBACK_FORWARD                     = 100086,
    SPELL_KNOCKBACK_BACK                        = 100087,
    SPELL_HEAD_BONK_R                           = 100089,
    SPELL_LEFT_SIDE_SMACK_R                     = 100090,
    SPELL_RIGHT_SIDE_SMACK_R                    = 100091,
    SPELL_TICKLE_R                              = 100092,
    SPELL_MOLTEN_BARRAGE_EFFECT_L               = 100071,
    SPELL_MOLTEN_BARRAGE_LEFT                   = 100072,
    SPELL_MOLTEN_BARRAGE_RIGHT                  = 100073,
    SPELL_MOLTEN_BARRAGE_EFFECT_R               = 100074,
    SPELL_MOLTEN_BARRAGE_VISUAL                 = 100075,
    SPELL_AGGRO_CLOSEST                         = 100462,
    SPELL_INVISIBILITY_AND_STEALTH_DETECTION    = 18950,

    // Egg Pile
    SPELL_SUMMON_SMOULDERING_HATCHLING          = 100096,
    SPELL_MOLTEN_EGG_TRASH_CALL_L               = 100097,
    SPELL_MOLTEN_EGG_TRASH_CALL_R               = 100098,
    SPELL_ALYSRAZOR_COSMETIC_EGG_XPLOSION       = 100099,

    // Volcanus
    SPELL_FLAMEWAKE                             = 100191,
};

enum Adds
{    
    NPC_MAGMAKIN        = 54144,
    NPC_MAGMA_CONDUIT   = 54145, // 97699, 98250, 100746

    NPC_STALKER         = 45979,

    NPC_LAVA            = 53585,
    NPC_MOLTEN_ERUPTER  = 53617,
    NPC_MOLTEN_ERUPTION = 53621,
    NPC_MOLTEN_SPEWER   = 53445,

};

enum Events
{
    // Ancient Core Hound
    EVENT_DINNER_TIME                   = 1,
    EVENT_FLAME_BREATH                  = 2,
    EVENT_TERRIFYING_ROAR               = 3,

    // Ancient Lava Dweller
    EVENT_LAVA_SHOWER                   = 4,

    // Fire Turtle Hatchling
    EVENT_SHELL_SPIN                    = 5,

    // Fire Archon
    EVENT_FIERY_TORMENT                 = 6,
    EVENT_FLAME_TORRENT                 = 7,

    // Molten Lord
    EVENT_MELT_ARMOR                    = 8,
    EVENT_FLAME_STOMP                   = 9,
    EVENT_SUMMON_LAVA_JETS              = 10,

    // Molten Flamefather
    EVENT_EARTHQUAKE                    = 11,
    EVENT_MAGMA_CONDUIT                 = 12,

    // Blazing Monstrosity
    EVENT_START_SPITTING                = 13,
    EVENT_CONTINUE_SPITTING             = 14,

    // Harbinger of Flame
    EVENT_FIEROBLAST                    = 15,
    EVENT_FIEROCLAST_BARRAGE            = 16,

    // Egg Pile
    EVENT_SUMMON_SMOULDERING_HATCHLING  = 17,

    // Volcanus
    EVENT_FLAMEWAKE                     = 18,
};

enum MiscData
{
    MODEL_INVISIBLE_STALKER     = 11686,
    ANIM_KIT_BIRD_WAKE          = 1469,
    ANIM_KIT_BIRD_TURN          = 1473,
};

class npc_firelands_ancient_core_hound : public CreatureScript
{
    public:
        npc_firelands_ancient_core_hound() : CreatureScript("npc_firelands_ancient_core_hound") { }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return GetInstanceAI<npc_firelands_ancient_core_houndAI>(pCreature);
        }

        struct npc_firelands_ancient_core_houndAI : public ScriptedAI
        {
            npc_firelands_ancient_core_houndAI(Creature* pCreature) : ScriptedAI(pCreature)
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
                events.ScheduleEvent(EVENT_DINNER_TIME, urand (15000, 20000));
                events.ScheduleEvent(EVENT_TERRIFYING_ROAR, urand(8000, 20000));
                events.ScheduleEvent(EVENT_FLAME_BREATH, urand(5000, 10000));
            }

            void UpdateAI(uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;
                
                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_DINNER_TIME:
                            DoCastVictim(SPELL_DINNER_TIME);
                            events.ScheduleEvent(EVENT_DINNER_TIME, urand(30000, 40000));
                            break;
                        case EVENT_FLAME_BREATH:
                            DoCastVictim(SPELL_FLAME_BREATH);
                            events.ScheduleEvent(EVENT_FLAME_BREATH, urand(15000, 20000));
                            break;
                        case EVENT_TERRIFYING_ROAR:
                            DoCast(me, SPELL_TERRIFYING_ROAR);
                            events.ScheduleEvent(EVENT_TERRIFYING_ROAR, urand(30000, 35000));
                            break;
                    }
                }
                
                DoMeleeAttackIfReady();
            }
        };
};

class npc_firelands_ancient_lava_dweller : public CreatureScript
{
    public:
        npc_firelands_ancient_lava_dweller() : CreatureScript("npc_firelands_ancient_lava_dweller") { }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return GetInstanceAI<npc_firelands_ancient_lava_dwellerAI>(pCreature);
        }

        struct npc_firelands_ancient_lava_dwellerAI : public Scripted_NoMovementAI
        {
            npc_firelands_ancient_lava_dwellerAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature)
            {
            }

            EventMap events;

            void Reset()
            {
                events.Reset();
            }

            void EnterCombat(Unit* /*who*/)
            {
                events.ScheduleEvent(EVENT_LAVA_SHOWER, urand(15000, 20000));
            }

            void UpdateAI(uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;
                
                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_LAVA_SHOWER:
                            DoCast(me, SPELL_LAVA_SHOWER);
                            events.ScheduleEvent(EVENT_LAVA_SHOWER, urand(45000, 55000));
                            break;
                    }
                }
                
                DoSpellAttackIfReady(SPELL_LAVA_SPIT);
            }
        };
};

class npc_firelands_fire_scorpion : public CreatureScript
{
    public:
        npc_firelands_fire_scorpion() : CreatureScript("npc_firelands_fire_scorpion") { }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return GetInstanceAI<npc_firelands_fire_scorpionAI>(pCreature);
        }

        struct npc_firelands_fire_scorpionAI : public ScriptedAI
        {
            npc_firelands_fire_scorpionAI(Creature* pCreature) : ScriptedAI(pCreature)
            {
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_SNARE, true);
            }

            void JustDied(Unit* killer)
            {
                DoCast(me, SPELL_FIERY_BLOOD);
            }
        };
};

class npc_firelands_fire_turtle_hatchling : public CreatureScript
{
    public:
        npc_firelands_fire_turtle_hatchling() : CreatureScript("npc_firelands_fire_turtle_hatchling") { }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return GetInstanceAI<npc_firelands_fire_turtle_hatchlingAI>(pCreature);
        }

        struct npc_firelands_fire_turtle_hatchlingAI : public ScriptedAI
        {
            npc_firelands_fire_turtle_hatchlingAI(Creature* pCreature) : ScriptedAI(pCreature)
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
                events.ScheduleEvent(EVENT_SHELL_SPIN, urand(10000, 20000));
            }

            void UpdateAI(uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;
                
                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_SHELL_SPIN:
                            DoCast(me, SPELL_SHELL_SPIN);
                            events.ScheduleEvent(EVENT_SHELL_SPIN, urand(35000, 50000));
                            break;
                    }
                }
                
                DoMeleeAttackIfReady();
            }
        };
};

class npc_firelands_flame_archon : public CreatureScript
{
    public:
        npc_firelands_flame_archon() : CreatureScript("npc_firelands_flame_archon") { }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return GetInstanceAI<npc_firelands_flame_archonAI>(pCreature);
        }

        struct npc_firelands_flame_archonAI : public ScriptedAI
        {
            npc_firelands_flame_archonAI(Creature* pCreature) : ScriptedAI(pCreature)
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
                events.ScheduleEvent(EVENT_FLAME_TORRENT, 10000);
                events.ScheduleEvent(EVENT_FIERY_TORMENT, 20000);
            }

            void UpdateAI(uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;
                
                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_FLAME_TORRENT:
                            DoCast(me, SPELL_FLAME_TORRENT);
                            events.ScheduleEvent(EVENT_FLAME_TORRENT, 40000);
                            break;
                        case EVENT_FIERY_TORMENT:
                            DoCast(me, SPELL_FIERY_TORMENT);
                            events.ScheduleEvent(EVENT_FIERY_TORMENT, 40000);
                            break;
                    }
                }
                
                DoMeleeAttackIfReady();
            }
        };
};

class npc_firelands_molten_lord : public CreatureScript
{
    public:
        npc_firelands_molten_lord() : CreatureScript("npc_firelands_molten_lord") { }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return GetInstanceAI<npc_firelands_molten_lordAI>(pCreature);
        }

        struct npc_firelands_molten_lordAI : public ScriptedAI
        {
            npc_firelands_molten_lordAI(Creature* pCreature) : ScriptedAI(pCreature)
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
                instance = pCreature->GetInstanceScript();
            }

            InstanceScript* instance;
            EventMap events;
            
            void Reset()
            {
                events.Reset();
            }

            void JustDied(Unit* killer)
            {
                if (instance)
                {
                    Map::PlayerList const& PlayerList = instance->instance->GetPlayers();
                    if (!PlayerList.isEmpty())
                    {
                        for (Map::PlayerList::const_iterator itr = PlayerList.begin(); itr != PlayerList.end(); ++itr)
                        {
                            if (Player* pPlayer = itr->getSource())
                            {
                                if (!pPlayer->IsAtGroupRewardDistance(me))
                                    continue;

                                // Only mages, warlocks priests, shamans and druids
                                // in caster specialization can accept quest
                                if (!(pPlayer->getClass() == CLASS_MAGE ||
                                    pPlayer->getClass() == CLASS_WARLOCK ||
                                    pPlayer->getClass() == CLASS_PRIEST ||
                                    (pPlayer->getClass() == CLASS_SHAMAN && (pPlayer->GetSpecializationId() == SPEC_SHAMAN_ELEMENTAL || pPlayer->GetSpecializationId() == SPEC_SHAMAN_RESTORATION)) ||
                                    (pPlayer->getClass() == CLASS_DRUID && (pPlayer->GetSpecializationId() == SPEC_DRUID_BALANCE || pPlayer->GetSpecializationId() == SPEC_DRUID_RESTORATION))))
                                    continue;

                                if (pPlayer->GetTeam() == ALLIANCE && pPlayer->GetQuestStatus(29453) == QUEST_STATUS_NONE)
                                    pPlayer->AddQuest(sQuestDataStore->GetQuestTemplate(29453), NULL);
                                else if (pPlayer->GetTeam() == HORDE && pPlayer->GetQuestStatus(29452) == QUEST_STATUS_NONE)
                                    pPlayer->AddQuest(sQuestDataStore->GetQuestTemplate(29452), NULL);
                            }
                        }
                    }
                }
            }

            void EnterCombat(Unit* /*who*/)
            {
                events.ScheduleEvent(EVENT_FLAME_STOMP, 5000);
                events.ScheduleEvent(EVENT_MELT_ARMOR, urand(3000, 7000));
                events.ScheduleEvent(EVENT_SUMMON_LAVA_JETS, 10000);
            }

            void UpdateAI(uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;
                
                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_FLAME_STOMP:
                            DoCast(me, SPELL_FLAME_STOMP);
                            events.ScheduleEvent(EVENT_FLAME_STOMP, urand(10000, 18000));
                            break;
                        case EVENT_MELT_ARMOR:
                            DoCastVictim(SPELL_MELT_ARMOR);
                            events.ScheduleEvent(EVENT_MELT_ARMOR, urand(7000, 14000));
                            break;
                        case EVENT_SUMMON_LAVA_JETS:
                            DoCast(me, SPELL_SUMMON_LAVA_JETS);
                            events.ScheduleEvent(EVENT_SUMMON_LAVA_JETS, urand(20000, 25000));
                            break;
                    }
                }
                
                DoMeleeAttackIfReady();
            }
        };
};

class npc_firelands_molten_flamefather : public CreatureScript
{
    public:
        npc_firelands_molten_flamefather() : CreatureScript("npc_firelands_molten_flamefather") { }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return GetInstanceAI<npc_firelands_molten_flamefatherAI>(pCreature);
        }

        struct npc_firelands_molten_flamefatherAI : public ScriptedAI
        {
            npc_firelands_molten_flamefatherAI(Creature* pCreature) : ScriptedAI(pCreature), summons(me)
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
            SummonList summons;

            void Reset()
            {
                events.Reset();
                summons.DespawnAll();
            }

            void EnterCombat(Unit* /*who*/)
            {
                events.ScheduleEvent(EVENT_EARTHQUAKE, urand(5000, 10000));
                events.ScheduleEvent(EVENT_MAGMA_CONDUIT, urand(6000, 7000));
            }

            void JustSummoned(Creature* summon)
            {
                summons.Summon(summon);
                if (me->isInCombat())
                    DoZoneInCombat(summon);
            }

            void SummonedCreatureDespawn(Creature* summon)
            {
                summons.Despawn(summon);
            }

            void UpdateAI(uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;
                
                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_EARTHQUAKE:
                            DoCastAOE(SPELL_EARTHQUAKE);
                            events.ScheduleEvent(EVENT_EARTHQUAKE, urand(10000, 15000));
                            break;
                        case EVENT_MAGMA_CONDUIT:
                            DoCastAOE(SPELL_MAGMA_CONDUIT);
                            break;
                    }
                }
                
                DoMeleeAttackIfReady();
            }
        };
};

class npc_firelands_magma_conduit : public CreatureScript
{
    public:
        npc_firelands_magma_conduit() : CreatureScript("npc_firelands_magma_conduit") { }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return GetInstanceAI<npc_firelands_magma_conduitAI>(pCreature);
        }

        struct npc_firelands_magma_conduitAI : public Scripted_NoMovementAI
        {
            npc_firelands_magma_conduitAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature), summons(me)
            {
                me->SetReactState(REACT_PASSIVE);
            }

            SummonList summons;

            void Reset()
            {
                summons.DespawnAll();
            }

            void JustSummoned(Creature* summon)
            {
                summons.Summon(summon);
                if (me->isInCombat())
                    DoZoneInCombat(summon);
            }

            void SummonedCreatureDespawn(Creature* summon)
            {
                summons.Despawn(summon);
            }

            void JustDied(Unit* killer)
            {
                me->DespawnOrUnsummon();
            }

            void UpdateAI(uint32 diff)
            {
                if (!UpdateVictim())
                {
                    me->DespawnOrUnsummon();
                    return;
                }
            }
        };
};

class npc_firelands_magmakin : public CreatureScript
{
    public:
        npc_firelands_magmakin() : CreatureScript("npc_firelands_magmakin") { }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return GetInstanceAI<npc_firelands_magmakinAI>(pCreature);
        }

        struct npc_firelands_magmakinAI : public ScriptedAI
        {
            npc_firelands_magmakinAI(Creature* pCreature) : ScriptedAI(pCreature)
            {
                me->SetSpeed(MOVE_RUN, 2.0f);
            }

            void UpdateAI(uint32 diff)
            {
                if (!UpdateVictim())
                {
                    me->DespawnOrUnsummon();
                    return;
                }

                if (me->GetDistance(me->getVictim()) < 2.0f)
                    DoCastAOE(SPELL_ERUPTION, true);                
            }
        };
};

class spell_firelands_ancient_core_hound_dinner_time : public SpellScriptLoader
{
    public:
        spell_firelands_ancient_core_hound_dinner_time() :  SpellScriptLoader("spell_firelands_ancient_core_hound_dinner_time") { }

        class spell_firelands_ancient_core_hound_dinner_time_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_firelands_ancient_core_hound_dinner_time_AuraScript);

            void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (!GetTarget())
                    return;

                GetTarget()->SetControlled(true, UNIT_STATE_STUNNED);
            }

            void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (!GetTarget())
                    return;

                GetTarget()->SetControlled(false, UNIT_STATE_STUNNED);                
            }

            void Register()
            {
                OnEffectApply += AuraEffectApplyFn(spell_firelands_ancient_core_hound_dinner_time_AuraScript::OnApply, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);
                OnEffectRemove += AuraEffectRemoveFn(spell_firelands_ancient_core_hound_dinner_time_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_firelands_ancient_core_hound_dinner_time_AuraScript();
        }
};

class spell_firelands_ancient_core_hound_flame_breath : public SpellScriptLoader
{
    public:
        spell_firelands_ancient_core_hound_flame_breath() :  SpellScriptLoader("spell_firelands_ancient_core_hound_flame_breath") { }

        class spell_firelands_ancient_core_hound_flame_breath_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_firelands_ancient_core_hound_flame_breath_AuraScript);

            void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (!GetTarget())
                    return;

                GetTarget()->SetControlled(true, UNIT_STATE_STUNNED);
            }

            void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (!GetTarget())
                    return;

                GetTarget()->SetControlled(false, UNIT_STATE_STUNNED);                
            }

            void Register()
            {
                OnEffectApply += AuraEffectApplyFn(spell_firelands_ancient_core_hound_flame_breath_AuraScript::OnApply, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
                OnEffectRemove += AuraEffectRemoveFn(spell_firelands_ancient_core_hound_flame_breath_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_firelands_ancient_core_hound_flame_breath_AuraScript();
        }
};

class spell_firelands_ancient_lava_dweller_lava_shower : public SpellScriptLoader
{
    public:
        spell_firelands_ancient_lava_dweller_lava_shower() : SpellScriptLoader("spell_firelands_ancient_lava_dweller_lava_shower") { }

        class spell_firelands_ancient_lava_dweller_lava_shower_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_firelands_ancient_lava_dweller_lava_shower_AuraScript);

            void PeriodicTick(AuraEffect const* aurEff)
            {
                if (!GetCaster())
                    return;

                if (GetCaster()->GetAI())
                    if (Unit* pTarget = GetCaster()->GetAI()->SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                        GetCaster()->CastSpell(pTarget, SPELL_LAVA_SHOWER_MISSILE, true);
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_firelands_ancient_lava_dweller_lava_shower_AuraScript::PeriodicTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_firelands_ancient_lava_dweller_lava_shower_AuraScript();
        }
};

class spell_firelands_fire_turtle_hatchling_shell_spin : public SpellScriptLoader
{
    public:
        spell_firelands_fire_turtle_hatchling_shell_spin() :  SpellScriptLoader("spell_firelands_fire_turtle_hatchling_shell_spin") { }

        class spell_firelands_fire_turtle_hatchling_shell_spin_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_firelands_fire_turtle_hatchling_shell_spin_AuraScript);

            void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (!GetTarget())
                    return;

                GetTarget()->SetControlled(true, UNIT_STATE_ROOT);
            }

            void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (!GetTarget())
                    return;

                GetTarget()->SetControlled(false, UNIT_STATE_ROOT);                
            }

            void Register()
            {
                OnEffectApply += AuraEffectApplyFn(spell_firelands_fire_turtle_hatchling_shell_spin_AuraScript::OnApply, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
                OnEffectRemove += AuraEffectRemoveFn(spell_firelands_fire_turtle_hatchling_shell_spin_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_firelands_fire_turtle_hatchling_shell_spin_AuraScript();
        }
};

class spell_firelands_flame_archon_fiery_torment : public SpellScriptLoader
{
    public:
        spell_firelands_flame_archon_fiery_torment() :  SpellScriptLoader("spell_firelands_flame_archon_fiery_torment") { }

        class spell_firelands_flame_archon_fiery_torment_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_firelands_flame_archon_fiery_torment_AuraScript);

            void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (!GetTarget())
                    return;

                GetTarget()->SetControlled(true, UNIT_STATE_STUNNED);
            }

            void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (!GetTarget())
                    return;

                GetTarget()->SetControlled(false, UNIT_STATE_STUNNED);                
            }

            void PeriodicTick(AuraEffect const* aurEff)
            {
                if (!GetCaster())
                    return;

                if (GetCaster()->GetAI())
                    if (Unit* pTarget = GetCaster()->GetAI()->SelectTarget(SELECT_TARGET_NEAREST, 0, 100.0f, true))
                        GetCaster()->CastSpell(pTarget, SPELL_FIERY_TORMENT_DMG, true);
            }


            void Register()
            {
                OnEffectApply += AuraEffectApplyFn(spell_firelands_flame_archon_fiery_torment_AuraScript::OnApply, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
                OnEffectRemove += AuraEffectRemoveFn(spell_firelands_flame_archon_fiery_torment_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_firelands_flame_archon_fiery_torment_AuraScript::PeriodicTick, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_firelands_flame_archon_fiery_torment_AuraScript();
        }
};

class spell_firelands_molten_lord_summon_lava_jets : public SpellScriptLoader
{
    public:
        spell_firelands_molten_lord_summon_lava_jets() : SpellScriptLoader("spell_firelands_molten_lord_summon_lava_jets") { }


        class spell_firelands_molten_lord_summon_lava_jets_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_firelands_molten_lord_summon_lava_jets_SpellScript);


            void HandleScript(SpellEffIndex /*effIndex*/)
            {
                if (!GetCaster() || !GetHitUnit())
                    return;

                GetCaster()->CastSpell(GetHitUnit(), 99538, true);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_firelands_molten_lord_summon_lava_jets_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_firelands_molten_lord_summon_lava_jets_SpellScript();
        }
};


enum Texts
{
    // Egg Pile
    EMOTE_CRACKING_EGGS         = 0,    // The Molten Eggs begin to crack and splinter!
};

#define SPELL_SHARE_HEALTH          (me->GetEntry() == NPC_BLAZING_MONSTROSITY_LEFT ? SPELL_SHARE_HEALTH_LEFT : SPELL_SHARE_HEALTH_RIGHT)
#define SPELL_MOLTEN_BARRAGE        (me->GetEntry() == NPC_BLAZING_MONSTROSITY_LEFT ? SPELL_MOLTEN_BARRAGE_LEFT : SPELL_MOLTEN_BARRAGE_RIGHT)
#define SPELL_MOLTEN_BARRAGE_EFFECT (me->GetEntry() == NPC_BLAZING_MONSTROSITY_LEFT ? SPELL_MOLTEN_BARRAGE_EFFECT_L : SPELL_MOLTEN_BARRAGE_EFFECT_R)

class RespawnEggEvent : public BasicEvent
{
    public:
        explicit RespawnEggEvent(Creature* egg) : _egg(egg) { }

        bool Execute(uint64 /*time*/, uint32 /*diff*/)
        {
            _egg->RestoreDisplayId();
            return true;
        }

    private:
        Creature* _egg;
};

class MoltenEggCheck
{
    public:
        explicit MoltenEggCheck(Creature* pile) : _eggPile(pile) { }

        bool operator()(Unit* object) const
        {
            if (object->GetEntry() != NPC_MOLTEN_EGG_TRASH)
                return false;

            if (object->GetDisplayId() != object->GetNativeDisplayId())
                return false;

            if (_eggPile->GetDistance2d(object) > 20.0f)
                return false;

            return true;
        }

    private:
        Creature* _eggPile;
};

class TrashRespawnWorker
{
    public:
        void operator()(Creature* creature) const
        {
            switch (creature->GetEntry())
            {
                case NPC_BLAZING_MONSTROSITY_LEFT:
                case NPC_BLAZING_MONSTROSITY_RIGHT:
                case NPC_EGG_PILE:
                case NPC_HARBINGER_OF_FLAME:
                case NPC_MOLTEN_EGG_TRASH:
                    if (!creature->isAlive())
                        creature->Respawn(true);
                    break;
                case NPC_SMOULDERING_HATCHLING:
                    creature->DespawnOrUnsummon();
                    break;
            }
        }
};

static void AlysrazorTrashEvaded(Creature* creature)
{
    TrashRespawnWorker check;
    Trinity::CreatureWorker<TrashRespawnWorker> worker(creature, check);
    Trinity::VisitNearbyGridObject(creature, SIZE_OF_GRIDS, worker);
}

class npc_harbinger_of_flame : public CreatureScript
{
    public:
        npc_harbinger_of_flame() : CreatureScript("npc_harbinger_of_flame") { }

        struct npc_harbinger_of_flameAI : public ScriptedAI
        {
            npc_harbinger_of_flameAI(Creature* creature) : ScriptedAI(creature)
            {
            }

            void EnterCombat(Unit* /*target*/)
            {
                //if (Creature* bird = ObjectAccessor::GetCreature(*me, me->GetGuidValue(UNIT_FIELD_CHANNEL_OBJECT)))
                //    DoZoneInCombat(bird, 200.0f);

                me->InterruptSpell(CURRENT_CHANNELED_SPELL);
                _events.Reset();
                _events.ScheduleEvent(EVENT_FIEROBLAST, 1);
                _events.ScheduleEvent(EVENT_FIEROCLAST_BARRAGE, 6000);
            }

            void JustReachedHome()
            {
                AlysrazorTrashEvaded(me);
            }

            void MoveInLineOfSight(Unit* unit)
            {
                if (me->isInCombat())
                    return;

                if (!unit->isCharmedOwnedByPlayerOrPlayer())
                    return;

                ScriptedAI::MoveInLineOfSight(unit);
            }

            void UpdateAI(uint32 diff)
            {
                if (!me->isInCombat())
                    if (!me->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
                        if (Creature* fireBird = me->FindNearestCreature((me->GetHomePosition().GetPositionY() > -275.0f ? NPC_BLAZING_MONSTROSITY_LEFT : NPC_BLAZING_MONSTROSITY_RIGHT), 100.0f))
                            DoCast(fireBird, SPELL_FIRE_CHANNELING);

                if (!UpdateVictim())
                    return;

                _events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = _events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_FIEROBLAST:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, false, -SPELL_RIDE_MONSTROSITY))
                                DoCast(target, SPELL_FIEROBLAST_TRASH);
                            _events.RescheduleEvent(EVENT_FIEROBLAST, 500);  // cast time is longer, but thanks to UNIT_STATE_CASTING check it won't trigger more often (need this because this creature gets a stacking haste aura)
                            break;
                        case EVENT_FIEROCLAST_BARRAGE:
                            DoCastAOE(SPELL_FIEROCLAST_BARRAGE);
                            _events.ScheduleEvent(EVENT_FIEROCLAST_BARRAGE, urand(9000, 12000));
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }

        private:
            EventMap _events;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return GetInstanceAI<npc_harbinger_of_flameAI>(creature);
        }
};

class npc_blazing_monstrosity : public CreatureScript
{
    public:
        npc_blazing_monstrosity() : CreatureScript("npc_blazing_monstrosity") { }

        struct npc_blazing_monstrosityAI : public PassiveAI
        {
            npc_blazing_monstrosityAI(Creature* creature) : PassiveAI(creature), _summons(creature)
            {
            }

            void EnterEvadeMode()
            {
                _summons.DespawnAll();
                _events.Reset();
                PassiveAI::EnterEvadeMode();
            }

            void JustDied(Unit* /*killer*/)
            {
                _summons.DespawnAll();
                _events.Reset();
            }

            void JustReachedHome()
            {
                AlysrazorTrashEvaded(me);
            }

            void EnterCombat(Unit* /*target*/)
            {
                DoZoneInCombat();
                me->RemoveAurasDueToSpell(SPELL_SLEEP_ULTRA_HIGH_PRIORITY);
                me->PlayOneShotAnimKit(ANIM_KIT_BIRD_WAKE);
                _events.Reset();
                _events.ScheduleEvent(EVENT_START_SPITTING, 6000);
                _events.ScheduleEvent(EVENT_CONTINUE_SPITTING, 9000);
            }

            void PassengerBoarded(Unit* passenger, int8 /*seat*/, bool apply)
            {
                if (!apply)
                    return;

                // Our passenger is another vehicle (boardable by players)
                DoCast(passenger, SPELL_SHARE_HEALTH, true);
                passenger->setFaction(35);
                passenger->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);

                // Hack to relocate vehicle on vehicle so exiting players are not moved under map
                Movement::MoveSplineInit init(*passenger);
                init.DisableTransportPathTransformations();
                init.MoveTo(0.6654003f, 0.0f, 1.9815f, false);
                init.SetFacing(0.0f);
                init.Launch();
            }

            void JustSummoned(Creature* summon)
            {
                _summons.Summon(summon);
            }

            void SummonedCreatureDespawn(Creature* summon)
            {
                _summons.Despawn(summon);
            }

            void UpdateAI(uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                _events.Update(diff);

                while (uint32 eventId = _events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_START_SPITTING:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, false, -SPELL_RIDE_MONSTROSITY))
                                DoCast(target, SPELL_MOLTEN_BARRAGE);
                            break;
                        case EVENT_CONTINUE_SPITTING:
                            DoCastAOE(SPELL_MOLTEN_BARRAGE_EFFECT);
                            if (Creature* egg = me->FindNearestCreature(NPC_EGG_PILE, 100.0f))
                                egg->AI()->DoAction(me->GetEntry());
                            break;
                    }
                }
            }

        private:
            SummonList _summons;
            EventMap _events;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return GetInstanceAI<npc_blazing_monstrosityAI>(creature);
        }
};

class npc_molten_barrage : public CreatureScript
{
    public:
        npc_molten_barrage() : CreatureScript("npc_molten_barrage") { }

        struct npc_molten_barrageAI : public NullCreatureAI
        {
            npc_molten_barrageAI(Creature* creature) : NullCreatureAI(creature)
            {
            }

            void AttackStart(Unit* target)
            {
                if (target)
                    me->GetMotionMaster()->MoveFollow(target, 0.0f, 0.0f, MOTION_SLOT_IDLE);
            }

            void IsSummonedBy(Unit* /*summoner*/)
            {
                DoCastAOE(SPELL_AGGRO_CLOSEST, true);
                DoCast(me, SPELL_MOLTEN_BARRAGE_VISUAL);
                DoCast(me, SPELL_INVISIBILITY_AND_STEALTH_DETECTION, true);
            }

            void MovementInform(uint32 movementType, uint32 /*pointId*/)
            {
                if (movementType != EFFECT_MOTION_TYPE)
                    return;

                DoCastAOE(SPELL_AGGRO_CLOSEST);
                me->ClearUnitState(UNIT_STATE_CANNOT_TURN);
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return GetInstanceAI<npc_molten_barrageAI>(creature);
        }
};

class npc_egg_pile : public CreatureScript
{
    public:
        npc_egg_pile() : CreatureScript("npc_egg_pile") { }

        struct npc_egg_pileAI : public CreatureAI
        {
            npc_egg_pileAI(Creature* creature) : CreatureAI(creature)
            {
            }

            void AttackStart(Unit* /*target*/) { }

            void Reset()
            {
                me->SetReactState(REACT_PASSIVE);
                _events.Reset();
                _callHatchlingSpell = 0;
            }

            void JustDied(Unit* /*killer*/)
            {
                _events.Reset();
                std::list<Creature*> eggs;
                GetCreatureListWithEntryInGrid(eggs, me, NPC_MOLTEN_EGG_TRASH, 20.0f);
                for (std::list<Creature*>::const_iterator itr = eggs.begin(); itr != eggs.end(); ++itr)
                    (*itr)->CastSpell(*itr, SPELL_ALYSRAZOR_COSMETIC_EGG_XPLOSION, TRIGGERED_FULL_MASK);

                DoCast(me, SPELL_ALYSRAZOR_COSMETIC_EGG_XPLOSION, true);
            }

            void JustReachedHome()
            {
                AlysrazorTrashEvaded(me);
            }

            void DoAction(int32 const action)
            {
                if (action != NPC_BLAZING_MONSTROSITY_LEFT &&
                    action != NPC_BLAZING_MONSTROSITY_RIGHT)
                    return;

                if (action == NPC_BLAZING_MONSTROSITY_LEFT)
                    Talk(EMOTE_CRACKING_EGGS);

                _callHatchlingSpell = (action == NPC_BLAZING_MONSTROSITY_LEFT) ? SPELL_MOLTEN_EGG_TRASH_CALL_L : SPELL_MOLTEN_EGG_TRASH_CALL_R;
                DoZoneInCombat();
                _events.Reset();
                _events.ScheduleEvent(EVENT_SUMMON_SMOULDERING_HATCHLING, 1);
            }

            void UpdateAI(uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                _events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = _events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_SUMMON_SMOULDERING_HATCHLING:
                        {
                            std::list<Creature*> eggs;
                            MoltenEggCheck check(me);
                            Trinity::CreatureListSearcher<MoltenEggCheck> searcher(me, eggs, check);
                            Trinity::VisitNearbyGridObject(me, 20.0f, searcher);
                            if (!eggs.empty())
                            {
                                Creature* egg = Trinity::Containers::SelectRandomContainerElement(eggs);
                                egg->CastSpell(egg, SPELL_SUMMON_SMOULDERING_HATCHLING, TRIGGERED_FULL_MASK);
                                egg->SetDisplayId(MODEL_INVISIBLE_STALKER);
                                egg->m_Events.AddEvent(new RespawnEggEvent(egg), egg->m_Events.CalculateTime(5000));
                            }

                            if (_callHatchlingSpell)
                                DoCastAOE(_callHatchlingSpell, true);
                            _events.ScheduleEvent(EVENT_SUMMON_SMOULDERING_HATCHLING, urand(6000, 10000));
                            break;
                        }
                        default:
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }

        private:
            EventMap _events;
            uint32 _callHatchlingSpell;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return GetInstanceAI<npc_egg_pileAI>(creature);
        }
};

class npc_firelands_dull_focus : public CreatureScript
{
    public:

        npc_firelands_dull_focus() : CreatureScript("npc_firelands_dull_focus") { }

        bool OnGossipHello(Player* pPlayer, Creature* pCreature)
        {
            InstanceScript* instance = pCreature->GetInstanceScript();
            if (!instance)
                return true;

            if (!pPlayer)
                return true;

            if (pPlayer->GetQuestStatus(29234) != QUEST_STATUS_INCOMPLETE)
                return true;

            uint32 spellId = 0;

            switch (pCreature->GetEntry())
            {
                case NPC_DULL_RHYOLITH_FOCUS: spellId = SPELL_CHARGED_RHYOLITH_FOCUS; break;
                case NPC_DULL_EMBERSTONE_FOCUS: spellId = SPELL_CHARGED_EMBERSTONE_FOCUS; break;
                case NPC_DULL_CHITINOUS_FOCUS: spellId = SPELL_CHARGED_CHITINOUS_FOCUS; break;
                case NPC_DULL_PYRESHELL_FOCUS: spellId = SPELL_CHARGED_PYRESHELL_FOCUS; break;
                default: break;
            }

            if (spellId)
            {
                pPlayer->CastSpell(pPlayer, spellId, true);
                pCreature->DespawnOrUnsummon();
            }
            return true;
        }
};

class npc_firelands_circle_of_thorns_portal : public CreatureScript
{
    public:
        npc_firelands_circle_of_thorns_portal() : CreatureScript("npc_firelands_circle_of_thorns_portal") { }

        bool OnGossipHello(Player* pPlayer, Creature* pCreature)
        {
            if (!pPlayer)
                return true;

            if (pCreature->GetAreaId() != 5798)
                pPlayer->NearTeleportTo(504.063416f, 476.256317f, 246.745483f, 2.30f, false);
            else
                pPlayer->NearTeleportTo(173.153091f, 283.155334f, 84.603622f, 3.69f, false); 
            return true;
        }
};

class npc_firelands_volcanus : public CreatureScript
{
    public:
        npc_firelands_volcanus() : CreatureScript("npc_firelands_volcanus") { }

        CreatureAI* GetAI(Creature* creature) const
        {
            return GetInstanceAI<npc_firelands_volcanusAI>(creature);
        }

        struct npc_firelands_volcanusAI : public CreatureAI
        {
            npc_firelands_volcanusAI(Creature* creature) : CreatureAI(creature)
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
                me->SetFloatValue(UNIT_FIELD_BOUNDING_RADIUS, 7);
                me->SetFloatValue(UNIT_FIELD_COMBAT_REACH, 7);
                events.Reset();
            }

            void EnterCombat(Unit* /*who*/)
            {
                events.ScheduleEvent(EVENT_FLAMEWAKE, 3000);
            }

            void JustDied(Unit* /*killer*/)
            {
                if (Creature* pStalker = me->SummonCreature(NPC_STALKER, me->GetHomePosition(), TEMPSUMMON_TIMED_DESPAWN, 10000))
                {
                    pStalker->RemoveAllAuras();
                    pStalker->CastSpell(pStalker, SPELL_BRANCH_OF_NORDRASSIL_WIN_COSMETIC);
                }
                me->DespawnOrUnsummon(500);
            }

            void UpdateAI(uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_FLAMEWAKE:
                            DoCastAOE(SPELL_FLAMEWAKE);
                            DoCast(me, SPELL_FIRE_IT_UP, true);
                            events.ScheduleEvent(EVENT_FLAMEWAKE, 10000);
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

class spell_alysrazor_cosmetic_egg_xplosion : public SpellScriptLoader
{
    public:
        spell_alysrazor_cosmetic_egg_xplosion() : SpellScriptLoader("spell_alysrazor_cosmetic_egg_xplosion") { }

        class spell_alysrazor_cosmetic_egg_xplosion_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_alysrazor_cosmetic_egg_xplosion_SpellScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sCreatureDisplayInfoStore.LookupEntry(MODEL_INVISIBLE_STALKER))
                    return false;
                return true;
            }

            void HandleExplosion(SpellEffIndex effIndex)
            {
                PreventHitDefaultEffect(effIndex);
                GetHitUnit()->SetDisplayId(MODEL_INVISIBLE_STALKER);
                if (Creature* creature = GetHitCreature())
                    creature->DespawnOrUnsummon(4000);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_alysrazor_cosmetic_egg_xplosion_SpellScript::HandleExplosion, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_alysrazor_cosmetic_egg_xplosion_SpellScript();
        }
};

class spell_alysrazor_turn_monstrosity : public SpellScriptLoader
{
    public:
        spell_alysrazor_turn_monstrosity() : SpellScriptLoader("spell_alysrazor_turn_monstrosity") { }

        class spell_alysrazor_turn_monstrosity_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_alysrazor_turn_monstrosity_SpellScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_GENERIC_DUMMY_CAST))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_KNOCKBACK_RIGHT))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_KNOCKBACK_LEFT))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_KNOCKBACK_FORWARD))
                    return false;
                if (!sSpellMgr->GetSpellInfo(SPELL_KNOCKBACK_BACK))
                    return false;
                return true;
            }

            void KnockBarrage(SpellEffIndex effIndex)
            {
                PreventHitDefaultEffect(effIndex);
                GetHitUnit()->GetMotionMaster()->MoveIdle();
                if (TempSummon* summ = GetHitUnit()->ToTempSummon())
                    if (Unit* summoner = summ->GetSummoner())
                        GetHitUnit()->CastSpell(summoner, SPELL_GENERIC_DUMMY_CAST, TRIGGERED_FULL_MASK);

                float angle = 0.0f;
                if (Unit* bird = GetCaster()->GetVehicleBase())
                {
                    bird->SetInFront(GetHitUnit());
                    angle = bird->GetOrientation();
                }

                uint32 spellId = 0;
                switch (GetSpellInfo()->Id)
                {
                    case SPELL_RIGHT_SIDE_SMACK_R:
                    case SPELL_RIGHT_SIDE_SMACK_L:
                        spellId = SPELL_KNOCKBACK_RIGHT;
                        angle -= M_PI * 0.5f;
                        break;
                    case SPELL_LEFT_SIDE_SMACK_R:
                    case SPELL_LEFT_SIDE_SMACK_L:
                        spellId = SPELL_KNOCKBACK_LEFT;
                        angle += M_PI * 0.5f;
                        break;
                    case SPELL_HEAD_BONK_R:
                    case SPELL_HEAD_BONK_L:
                        spellId = SPELL_KNOCKBACK_FORWARD;
                        break;
                    case SPELL_TICKLE_R:
                    case SPELL_TICKLE_L:
                        spellId = SPELL_KNOCKBACK_BACK;
                        angle -= M_PI;
                        break;
                }

                // Cannot wait for object update to process facing spline, it's needed in next spell cast
                GetHitUnit()->SetOrientation(angle);
                GetHitUnit()->SetFacingTo(angle);
                GetHitUnit()->AddUnitState(UNIT_STATE_CANNOT_TURN);
                GetHitUnit()->CastSpell(GetHitUnit(), spellId, TRIGGERED_FULL_MASK);
            }

            void TurnBird(SpellEffIndex effIndex)
            {
                PreventHitDefaultEffect(effIndex);
                GetHitUnit()->PlayOneShotAnimKit(ANIM_KIT_BIRD_TURN);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_alysrazor_turn_monstrosity_SpellScript::KnockBarrage, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
                OnEffectHitTarget += SpellEffectFn(spell_alysrazor_turn_monstrosity_SpellScript::TurnBird, EFFECT_1, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_alysrazor_turn_monstrosity_SpellScript();
        }
};

class spell_alysrazor_aggro_closest : public SpellScriptLoader
{
    public:
        spell_alysrazor_aggro_closest() : SpellScriptLoader("spell_alysrazor_aggro_closest") { }

        class spell_alysrazor_aggro_closest_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_alysrazor_aggro_closest_SpellScript);

            bool Load()
            {
                return GetCaster()->GetTypeId() == TYPEID_UNIT;
            }

            void HandleEffect(SpellEffIndex effIndex)
            {
                PreventHitDefaultEffect(effIndex);
                float curThreat = GetCaster()->getThreatManager().getThreat(GetHitUnit(), true);
                GetCaster()->getThreatManager().addThreat(GetHitUnit(), -curThreat + 50000.0f / std::min(1.0f, GetCaster()->GetDistance(GetHitUnit())));
            }

            void UpdateThreat()
            {
                GetCaster()->ClearUnitState(UNIT_STATE_CASTING);
                GetCaster()->GetAI()->AttackStart(GetCaster()->ToCreature()->SelectVictim());
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_alysrazor_aggro_closest_SpellScript::HandleEffect, EFFECT_0, SPELL_EFFECT_DUMMY);
                AfterCast += SpellCastFn(spell_alysrazor_aggro_closest_SpellScript::UpdateThreat);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_alysrazor_aggro_closest_SpellScript();
        }
};

class spell_firelands_siphon_essence : public SpellScriptLoader
{
    public:
        spell_firelands_siphon_essence() : SpellScriptLoader("spell_firelands_siphon_essence") { }

        class spell_firelands_siphon_essence_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_firelands_siphon_essence_SpellScript);

            void HandleScript(SpellEffIndex effIndex)
            {
                if (!GetCaster())
                    return;

                GetCaster()->CastSpell(GetCaster(), SPELL_SIPHON_ESSENCE_CREDIT, true);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_firelands_siphon_essence_SpellScript::HandleScript, EFFECT_1, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_firelands_siphon_essence_SpellScript();
        }
};

class npc_firelands_instance_portal : public CreatureScript
{
public:
    npc_firelands_instance_portal() : CreatureScript("npc_firelands_instance_portal") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return GetInstanceAI<npc_firelands_instance_portalAI>(creature);
    }

    struct npc_firelands_instance_portalAI : public CreatureAI
    {
        npc_firelands_instance_portalAI(Creature* creature) : CreatureAI(creature) { }

        void OnSpellClick(Unit* clicker)
        {
            if (InstanceScript* instance = me->GetInstanceScript())
                if (instance->GetBossState(DATA_BALEROC) != DONE)
                    return;

            if (me->GetEntry() == 54348)
                clicker->NearTeleportTo(362.498f, -97.7795f, 78.3288f, 3.64774f, false);
            else if (me->GetEntry() == 54367)
                clicker->NearTeleportTo(-359.944f, 206.012f, 52.32f, 3.64774f, false);
        }

        void UpdateAI(uint32 diff) { }
    };
};

void AddSC_firelands()
{
    new npc_firelands_ancient_core_hound();
    new npc_firelands_ancient_lava_dweller();
    new npc_firelands_fire_scorpion();
    new npc_firelands_fire_turtle_hatchling();
    new npc_firelands_flame_archon();
    new npc_firelands_molten_lord();
    new npc_firelands_molten_flamefather();
    new npc_firelands_magma_conduit();
    new npc_firelands_magmakin();

    new spell_firelands_ancient_core_hound_dinner_time();
    new spell_firelands_ancient_core_hound_flame_breath();
    new spell_firelands_ancient_lava_dweller_lava_shower();
    new spell_firelands_fire_turtle_hatchling_shell_spin();
    new spell_firelands_flame_archon_fiery_torment();
    new spell_firelands_molten_lord_summon_lava_jets();

    // alysrazor event
    /*new npc_harbinger_of_flame();
    new npc_blazing_monstrosity();
    new npc_molten_barrage();
    new npc_egg_pile();
    new spell_alysrazor_cosmetic_egg_xplosion();
    new spell_alysrazor_turn_monstrosity();
    new spell_alysrazor_aggro_closest();*/

    new npc_firelands_dull_focus();
    new npc_firelands_circle_of_thorns_portal();
    new npc_firelands_volcanus();
    new spell_firelands_siphon_essence();
    new npc_firelands_instance_portal();
}