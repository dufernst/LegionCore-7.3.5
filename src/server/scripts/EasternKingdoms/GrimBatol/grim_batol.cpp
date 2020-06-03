#include "grim_batol.h"
    
enum Creatures
{
    NPC_ASCENDED_FLAMESEEKER    = 39415,
    NPC_ASCENDED_ROCKBREAKER    = 40272,
    NPC_FISSURE                 = 41091,
    NPC_ASCENDED_WATERLASHER    = 40273,
    NPC_ASCENDED_WINDWALKER     = 39414,
    NPC_AZUREBORNE_GUARDIAN     = 39854,
    NPC_AZUREBORNE_SEER_1       = 39855,
    NPC_AZUREBORNE_SEER_2       = 40291,
    NPC_AZUREBORNE_WARLORD      = 39909,
    NPC_EMPOWERING_FLAMES       = 41045,
    NPC_ENSLAVED_BURNING_EMBER  = 39892,
    NPC_ENSLAVED_GRONN_BRUTE    = 40166,
    NPC_ENSLAVED_ROCK_ELEMENTAL = 39900,
    NPC_ENSLAVED_THUNDER_SPIRIT = 40269,
    NPC_ENSLAVED_WATER_SPIRIT   = 39961,
    NPC_FACELESS_CORRUPTOR      = 39392,
    NPC_HOOKED_NET              = 48756,
    NPC_NET                     = 42570,
    NPC_TROGG_DWELLER           = 39450,
    NPC_TWILIGHT_ARMSMASTER_1   = 40306,
    NPC_TWILIGHT_ARMSMASTER_2   = 41073,
    NPC_TWILIGHT_BEGUILER       = 40167,
    NPC_TWILIGHT_DRAKE_1        = 41095,
    NPC_TWILIGHT_DRAKE_2        = 39390,
    NPC_TWILIGHT_EARTHSHAPER    = 39890,
    NPC_TWILIGHT_ENFORCER_1     = 40448,
    NPC_TWILIGHT_ENFORCER_2     = 39956,
    NPC_TWILIGHT_FIRECATCHER    = 39870,
    NPC_TWILIGHT_SHADOW_WEAVER  = 39954,
    NPC_KHAAPHOM                = 40953,
    NPC_TWILIGHT_STORMBREAKER   = 39962,
    NPC_TWILIGHT_THUNDERCALLER  = 40270,
    NPC_TWILIGHT_WAR_MAGE       = 40268,
    NPC_TWILIGHT_WYRMCALLER     = 39873,
    NPC_TWISTED_VISAGE          = 41040,

    
    NPC_FARSEER_THOORANU        = 50385,
    NPC_VELASTRASZA             = 50390,
    NPC_BALEFLAME               = 50387
};

// 94350 summon red drake
enum Spells
{
    //ascended flameseeker
    SPELL_CONFOUNDING_FLAMES        = 76514,
    SPELL_ERUPTING_FIRE             = 76517,

    //ascended rockbreaker
    SPELL_BURNING_FISTS             = 76086,
    SPELL_PETRIFIED_SKIN            = 76792,
    SPELL_ROCK_SMASH                = 76779,
    SPELL_ROCK_SMASH_DMG            = 76782,
    SPELL_FISSURE_TRIGGER           = 76785,
    SPELL_FISSURE_DMG               = 76786,

    //ascended waterlasher
    SPELL_FOCUSED_GAYSER            = 76797,
    SPELL_ABSORB_THUNDER            = 76095,
    SPELL_LIGHTNING_CLOUD           = 76097,
    SPELL_LIGHTNING_STRIKE_DMG      = 76101,
    SPELL_WATER_SPOUT               = 76794,

    //ascended windwalker
    SPELL_ABSORB_WATER              = 76029,
    SPELL_WATER_INFUSIED_BLADES     = 76036,
    SPELL_TSUNAMI                   = 76045,
    SPELL_WINDWALK                  = 76557,

    //azureborne guardian
    SPELL_ARCANE_INFUSION           = 76378,
    SPELL_ARCANE_SLASH              = 76392,
    SPELL_CURSE_OF_THE_AZUREBORNE   = 76394,

    //azoreborne seer
    SPELL_TWILIGHT_BOLT             = 76369,
    SPELL_TWISTED_ARCANE_TRIGGER    = 79446,
    SPELL_TWISTED_ARCANE            = 76340,
    SPELL_WARPED_TWILIGHT           = 76370,
    SPELL_WARPED_TWILIGHT_DUMMY     = 76373,

    //azureborne warlord
    SPELL_AZURE_BLAST               = 76620,
    SPELL_CONJURE_TWISTED_VISAGE    = 76626,

    //crimsonborne guardian
    SPELL_CRIMSON_CHARGE            = 76404,
    SPELL_CRIMSON_SHOCKWAVE         = 76409,

    //crimsonborne seer
    SPELL_BLAZING_TWILIGHT_SHIELD   = 76314,
    SPELL_BLAZE                     = 76327,
    SPELL_CORRUPTED_FLAME           = 76332,
    
    //crimsonborne warlord
    SPELL_DISARMING_BLAZE           = 76679,
    SPELL_EMPOWERING_TWILIGHT       = 76685,
    SPELL_EMPOWERING_TWILIGHT_AURA  = 76692,
    SPELL_EMPOWERING_TWILIGHT_DMG   = 76693,

    //enslaved burning ember
    SPELL_FLAME_SHOCK               = 90846,

    //enslaved gronn brute
    SPELL_CRUNCH_ARMOR              = 76703,

    //enslaved rock elemental
    SPELL_JAGGED_ROCK_SHIELD        = 76014,

    //faceless corruptor
    SPELL_SIPHON_ESSENSE            = 75755,

    //trogg dweller
    SPELL_CLAW_PUNCTURE             = 76507,

    //twilight armsmaster
    SPELL_FLURRY_OF_BLOWS           = 76729,
    SPELL_MORTAL_STRIKE             = 76727,

    //twilight beguiler
    SPELL_CHAINED_MIND              = 76711,
    SPELL_DECEITFUL_BLAST           = 76715,

    //twilight drake                
    SPELL_TWILIGHT_BREATH           = 76817,

    //twilight earthshaper
    SPELL_EARTH_SPIKE               = 76603,
    SPELL_STONE_SKIN                = 76596,
    SPELL_SUMMON_ROCK_ELEMENTAL     = 74552,

    //twilight enforcer
    SPELL_DIZZY                     = 76415,
    SPELL_MEAT_GRINDER              = 76411,
    SPELL_MEAT_GRINDER_DMG          = 76413,
    SPELL_MEAT_GRINDER_DMG_H        = 90664,
    SPELL_MEAT_GRINDER_TRIGGER      = 76414,

    //twilight firecatcher
    SPELL_MOLTEN_BLAST              = 76765,
    SPELL_FLAME_CONDUIT             = 76766,
    SPELL_FLAME_CONDUIT_DMG         = 76768,
    SPELL_SUMMON_BURNING_EMBER      = 74551,

    //twilight shadow weaver
    SPELL_SHADOW_BOLT               = 76416,
    SPELL_SHADOW_WEAVE_SCRIPT       = 90673,
    SPELL_SHADOW_WEAVE_DUMMY        = 90674,
    SPELL_SHADOW_WEAVE_DMG          = 90678,
    SPELL_SUMMON_FELHUNTER          = 76418,
    SPELL_SPELL_LOCK                = 40953,

    //twilight stormbreaker
    SPELL_WATER_BOLT                = 76720,
    SPELL_WATER_SHELL               = 90522,
    SPELL_SUMMON_WATER_SPIRIT       = 74561,

    //twilight thunderbreaker
    SPELL_CHAIN_LIGHTNING           = 76578, //5m
    SPELL_OVERCHARGE                = 76579,
    SPELL_ELECTRIC_BLAST            = 82973, //+dmg
    SPELL_SUMMON_THUNDER_SPIRIT     = 75096,

    //twilight war-mage
    SPELL_FIRE_ENCHANT              = 76822,
    SPELL_ICE_ENCHANT               = 76823,
    SPELL_POLYMORPH                 = 76826,

    //twilight wyrmcaller
    SPELL_FEED_PET                  = 76816,
};

enum Events
{
    EVENT_CONFOUNDING_FLAMES        = 1,
    EVENT_ERUPTING_FIRE             = 2,
    EVENT_BURNING_FISTS             = 3,
    EVENT_PETRIFIED_SKIN            = 4,
    EVENT_ROCK_SMASH                = 5,
    EVENT_FOCUSED_GAYSER            = 6,
    EVENT_ABSORB_THUNDER            = 7,
    EVENT_WATER_SPOUT               = 8,
    EVENT_ABSORB_WATER              = 9,
    EVENT_ARCANE_INFUSION           = 10,
    EVENT_CURSE_OF_THE_AZUREBORNE   = 11,
    EVENT_TWILIGHT_BOLT             = 12,
    EVENT_WARPED_TWILIGHT           = 13,
    EVENT_AZURE_BLAST               = 14,
    EVENT_CONJURE_TWISTED_VISAGE    = 15,
    EVENT_CRIMSON_CHARGE            = 16,
    EVENT_BLAZING_TWILIGHT_SHIELD   = 17,
    EVENT_CORRUPTED_FLAME           = 18,
    EVENT_DISARMING_BLAZE           = 19,
    EVENT_EMPOWERING_TWILIGHT       = 20,
    EVENT_FLAME_SHOCK               = 21,
    EVENT_JAGGED_ROCK_SHIELD        = 22,
    EVENT_SIPHON_ESSENSE            = 23,
    EVENT_CLAW_PUNCTURE             = 24,
    EVENT_MORTAL_STRIKE             = 25,
    EVENT_FLURRY_OF_BLOWS           = 26,
    EVENT_TWILIGHT_BREATH           = 27,
    EVENT_STONE_SKIN                = 28,
    EVENT_EARTH_SPIKE               = 29,
    EVENT_CHAINED_MIND              = 30,
    EVENT_DECEITFUL_BLAST           = 31,
    EVENT_CRUNCH_ARMOR              = 32,
    EVENT_MEAT_GRINDER              = 33,
    EVENT_FLAME_CONDUIT             = 34,
    EVENT_MOLTEN_BLAST              = 35,
    EVENT_CALL_WYRM                 = 36,
    EVENT_ENCHANT                   = 37,
    EVENT_POLYMORPH                 = 38,
    EVENT_FEED_PET                  = 39,
    EVENT_SHADOW_BOLT               = 40,
    EVENT_SHADOW_WEAVE              = 41,
    EVENT_WATER_BOLT                = 42,
    EVENT_WATER_SHELL               = 43,
    EVENT_CHAIN_LIGHTNING           = 44,
    EVENT_OVERCHARGE                = 45

};

class npc_ascended_flameseeker : public CreatureScript
{
    public:
        npc_ascended_flameseeker() : CreatureScript("npc_ascended_flameseeker") {}

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_ascended_flameseekerAI(pCreature);
        }

        struct npc_ascended_flameseekerAI : public ScriptedAI
        {
            npc_ascended_flameseekerAI(Creature *c) : ScriptedAI(c) {}
            
            EventMap events;

            void Reset() override
            {
                events.Reset();
            }

            void EnterCombat(Unit* /*attacker*/) override
            {
                events.RescheduleEvent(EVENT_CONFOUNDING_FLAMES, urand(5000, 10000));
                events.RescheduleEvent(EVENT_ERUPTING_FIRE, urand(7000, 12000));
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
                    case EVENT_CONFOUNDING_FLAMES:
                        if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                            DoCast(target, SPELL_CONFOUNDING_FLAMES, false);
                        events.RescheduleEvent(EVENT_CONFOUNDING_FLAMES, urand(7000, 12000));
                        break;
                    case EVENT_ERUPTING_FIRE:
                        if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                            DoCast(target, SPELL_CONFOUNDING_FLAMES, false);
                        events.RescheduleEvent(EVENT_ERUPTING_FIRE, urand(9000, 15000));
                        break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };
};

class npc_ascended_rockbreaker : public CreatureScript
{
    public:
        npc_ascended_rockbreaker() : CreatureScript("npc_ascended_rockbreaker") {}

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_ascended_rockbreakerAI(pCreature);
        }

        struct npc_ascended_rockbreakerAI : public ScriptedAI
        {
            npc_ascended_rockbreakerAI(Creature *c) : ScriptedAI(c) {}
            
            EventMap events;

            void Reset() override
            {
                events.Reset();
            }

            void EnterCombat(Unit* /*attacker*/) override
            {
                DoCast(SPELL_BURNING_FISTS);
                events.RescheduleEvent(EVENT_BURNING_FISTS, 45000);
                events.RescheduleEvent(EVENT_PETRIFIED_SKIN, urand(5000, 7000));
                events.RescheduleEvent(EVENT_ROCK_SMASH, urand(8000, 12000));
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
                    switch(eventId)
                    {
                    case EVENT_BURNING_FISTS:
                        DoCast(SPELL_BURNING_FISTS);
                        events.RescheduleEvent(EVENT_BURNING_FISTS, 45000);
                        break;
                    case EVENT_PETRIFIED_SKIN:
                        if (auto victim = me->getVictim())
                            DoCast(victim, SPELL_PETRIFIED_SKIN, false);
                        events.RescheduleEvent(EVENT_PETRIFIED_SKIN, urand(8000, 12000));
                        break;
                    case EVENT_ROCK_SMASH:
                        if (auto target = SelectTarget(SELECT_TARGET_FARTHEST, 0, 100.0f, true))
                            DoCast(target, SPELL_ROCK_SMASH, false);
                        events.RescheduleEvent(EVENT_ROCK_SMASH, urand(9000, 14000));
                        break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };
};

class npc_ascended_waterlasher : public CreatureScript
{
    public:
        npc_ascended_waterlasher() : CreatureScript("npc_ascended_waterlasher") {}

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_ascended_waterlasherAI(pCreature);
        }

        struct npc_ascended_waterlasherAI : public ScriptedAI
        {
            npc_ascended_waterlasherAI(Creature *c) : ScriptedAI(c) {}
            
            EventMap events;

            void Reset() override
            {
                events.Reset();
            }

            void EnterCombat(Unit* /*attacker*/) override
            {
                events.RescheduleEvent(EVENT_ABSORB_THUNDER, urand(25000, 30000));
                events.RescheduleEvent(EVENT_FOCUSED_GAYSER, urand(5000, 9000));
                events.RescheduleEvent(EVENT_WATER_SPOUT, urand(3000, 10000));
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
                    switch(eventId)
                    {
                    case EVENT_FOCUSED_GAYSER:
                        DoCast(SPELL_FOCUSED_GAYSER);
                        events.RescheduleEvent(EVENT_FOCUSED_GAYSER, urand(15000, 20000));
                        break;
                    case EVENT_WATER_SPOUT:
                        if (auto victim = me->getVictim())
                            DoCast(victim, SPELL_WATER_SPOUT, false);
                        events.RescheduleEvent(EVENT_WATER_SPOUT, urand(15000, 20000));
                        break;
                    case EVENT_ABSORB_THUNDER:
                        if (auto _windwalker = me->FindNearestCreature(NPC_ASCENDED_WINDWALKER, 100.0f, true))
                            DoCast(_windwalker, SPELL_ABSORB_THUNDER, false);
                        break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };
};

class npc_ascended_windwalker: public CreatureScript
{
    public:
        npc_ascended_windwalker() : CreatureScript("npc_ascended_windwalker") {}

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_ascended_windwalkerAI(pCreature);
        }

        struct npc_ascended_windwalkerAI : public ScriptedAI
        {
            npc_ascended_windwalkerAI(Creature *c) : ScriptedAI(c) {}
            
            EventMap events;

            void Reset() override
            {
                events.Reset();
            }

            void EnterCombat(Unit* /*attacker*/) override
            {
                events.RescheduleEvent(EVENT_ABSORB_WATER, urand(25000, 30000));
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
                    switch(eventId)
                    {
                    case EVENT_ABSORB_WATER:
                        if (auto _waterlasher = me->FindNearestCreature(NPC_ASCENDED_WATERLASHER, 100.0f, true))
                            DoCast(_waterlasher, SPELL_ABSORB_WATER, false);
                        break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };
};

class npc_azureborne_guardian: public CreatureScript
{
    public:
        npc_azureborne_guardian() : CreatureScript("npc_azureborne_guardian") {}

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_azureborne_guardianAI(pCreature);
        }

        struct npc_azureborne_guardianAI : public ScriptedAI
        {
            npc_azureborne_guardianAI(Creature *c) : ScriptedAI(c) {}
            
            EventMap events;

            void Reset() override
            {
                events.Reset();
            }

            void EnterCombat(Unit* /*attacker*/) override
            {
                events.RescheduleEvent(EVENT_ARCANE_INFUSION, urand(2000, 4000));
                events.RescheduleEvent(EVENT_CURSE_OF_THE_AZUREBORNE, urand(6000, 7000));
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
                    switch(eventId)
                    {
                    case EVENT_ARCANE_INFUSION:
                        DoCast(SPELL_ARCANE_INFUSION);
                        events.RescheduleEvent(EVENT_ARCANE_INFUSION, urand(15000, 20000));
                        break;
                    case EVENT_CURSE_OF_THE_AZUREBORNE:
                        if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                            DoCast(target, SPELL_CURSE_OF_THE_AZUREBORNE, false);
                        events.RescheduleEvent(EVENT_CURSE_OF_THE_AZUREBORNE, urand(10000, 15000));
                        break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };
};

class npc_azureborne_seer: public CreatureScript
{
    public:
        npc_azureborne_seer() : CreatureScript("npc_azureborne_seer") {}

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_azureborne_seerAI(pCreature);
        }

        struct npc_azureborne_seerAI : public ScriptedAI
        {
            npc_azureborne_seerAI(Creature *c) : ScriptedAI(c) {}
            
            EventMap events;

            void Reset() override
            {
                events.Reset();
            }

            void EnterCombat(Unit* /*attacker*/)
            {
                events.RescheduleEvent(EVENT_WARPED_TWILIGHT, urand(15000, 20000));
                events.RescheduleEvent(EVENT_TWILIGHT_BOLT, urand(1000, 2000));
            }

            void UpdateAI(uint32 diff)
            {
                if (!me->HasAura(SPELL_TWISTED_ARCANE_TRIGGER) && !me->HasAura(SPELL_TWISTED_ARCANE))
                {
                    DoCast(SPELL_TWISTED_ARCANE_TRIGGER);
                    return;
                }

                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch(eventId)
                    {
                    case EVENT_TWILIGHT_BOLT:
                        if (auto victim = me->getVictim())
                            DoCast(victim, SPELL_TWILIGHT_BOLT, false);
                        events.RescheduleEvent(EVENT_TWILIGHT_BOLT, 3500);
                        break;
                    case EVENT_WARPED_TWILIGHT:
                        DoCast(SPELL_WARPED_TWILIGHT);
                        events.RescheduleEvent(EVENT_WARPED_TWILIGHT, urand(20000, 30000));
                        break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };
};

class npc_azureborne_warlord: public CreatureScript
{
    public:
        npc_azureborne_warlord() : CreatureScript("npc_azureborne_warlord") {}

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_azureborne_warlordAI(pCreature);
        }

        struct npc_azureborne_warlordAI : public ScriptedAI
        {
            npc_azureborne_warlordAI(Creature *c) : ScriptedAI(c) {}
            
            EventMap events;

            void Reset() override
            {
                events.Reset();
            }

            void EnterCombat(Unit* /*attacker*/) override
            {
                events.RescheduleEvent(EVENT_AZURE_BLAST, urand(5000, 10000));
                events.RescheduleEvent(EVENT_CONJURE_TWISTED_VISAGE, urand(7000, 12000));
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
                    case EVENT_AZURE_BLAST:
                        DoCast(SPELL_AZURE_BLAST);
                        events.RescheduleEvent(EVENT_AZURE_BLAST, urand(10000, 15000));
                        break;
                    case EVENT_CONJURE_TWISTED_VISAGE:
                        DoCast(SPELL_CONJURE_TWISTED_VISAGE);
                        events.RescheduleEvent(EVENT_CONJURE_TWISTED_VISAGE, urand(15000, 20000));
                        break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };
};

class npc_crimsonborne_guardian: public CreatureScript
{
    public:
        npc_crimsonborne_guardian() : CreatureScript("npc_crimsonborne_guardian") {}

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_crimsonborne_guardianAI(pCreature);
        }

        struct npc_crimsonborne_guardianAI : public ScriptedAI
        {
            npc_crimsonborne_guardianAI(Creature *c) : ScriptedAI(c) {}

            EventMap events;

            void Reset() override
            {
                events.Reset();
            }

            void EnterCombat(Unit* /*attacker*/) override
            {
                events.RescheduleEvent(EVENT_CRIMSON_CHARGE, urand(3000, 5000));
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
                    case EVENT_CRIMSON_CHARGE:
                        if (auto target = SelectTarget(SELECT_TARGET_FARTHEST, 0, 100.0f, true))
                            DoCast(target, SPELL_CRIMSON_CHARGE, false);
                        events.RescheduleEvent(EVENT_CRIMSON_CHARGE, urand(10000, 15000));
                        break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };
};

class npc_crimsonborne_seer: public CreatureScript
{
    public:
        npc_crimsonborne_seer() : CreatureScript("npc_crimsonborne_seer") {}

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_crimsonborne_seerAI(pCreature);
        }

        struct npc_crimsonborne_seerAI : public ScriptedAI
        {
            npc_crimsonborne_seerAI(Creature *c) : ScriptedAI(c) {}
            
            EventMap events;

            void Reset() override
            {
                events.Reset();
            }

            void EnterCombat(Unit* /*attacker*/) override
            {
                events.RescheduleEvent(EVENT_BLAZING_TWILIGHT_SHIELD, urand(2000, 5000));
                events.RescheduleEvent(EVENT_CORRUPTED_FLAME, urand(6000, 10000));
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
                    switch(eventId)
                    {
                    case EVENT_BLAZING_TWILIGHT_SHIELD:
                        DoCast(SPELL_BLAZING_TWILIGHT_SHIELD);
                        events.RescheduleEvent(EVENT_BLAZING_TWILIGHT_SHIELD, urand(12000, 15000));
                        break;
                    case EVENT_CORRUPTED_FLAME:
                        if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                            DoCast(target, SPELL_CORRUPTED_FLAME, false);
                        events.RescheduleEvent(EVENT_CORRUPTED_FLAME, urand(8000, 14000));
                        break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };
};

class npc_crimsonborne_warlord: public CreatureScript
{
    public:
        npc_crimsonborne_warlord() : CreatureScript("npc_crimsonborne_warlord") {}

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_crimsonborne_warlordAI(pCreature);
        }

        struct npc_crimsonborne_warlordAI : public ScriptedAI
        {
            npc_crimsonborne_warlordAI(Creature *c) : ScriptedAI(c) {}
            
            EventMap events;

            void Reset() override
            {
                events.Reset();
            }

            void EnterCombat(Unit* /*attacker*/) override
            {
                events.RescheduleEvent(EVENT_DISARMING_BLAZE, urand(3000, 5000));
                events.RescheduleEvent(EVENT_EMPOWERING_TWILIGHT, urand(9000, 12000));
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
                    switch(eventId)
                    {
                    case EVENT_DISARMING_BLAZE:
                        if (auto victim = me->getVictim())
                            DoCast(victim, SPELL_DISARMING_BLAZE, false);
                        events.RescheduleEvent(EVENT_DISARMING_BLAZE, urand(10000, 12000));
                        break;
                    case EVENT_EMPOWERING_TWILIGHT:
                        DoCast(SPELL_EMPOWERING_TWILIGHT);
                        events.RescheduleEvent(EVENT_EMPOWERING_TWILIGHT, urand(25000, 30000));
                        break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };
};

class npc_enslaved_burning_ember: public CreatureScript
{
    public:
        npc_enslaved_burning_ember() : CreatureScript("npc_enslaved_burning_ember") {}

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_enslaved_burning_emberAI(pCreature);
        }

        struct npc_enslaved_burning_emberAI : public ScriptedAI
        {
            npc_enslaved_burning_emberAI(Creature *c) : ScriptedAI(c) {}
            
            EventMap events;

            void Reset() override
            {
                events.Reset();
            }

            void EnterCombat(Unit* /*attacker*/) override
            {
                events.RescheduleEvent(EVENT_FLAME_SHOCK, urand(3000, 4000));
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while(uint32 eventId = events.ExecuteEvent())
                {
                    switch(eventId)
                    {
                    case EVENT_FLAME_SHOCK:
                        if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                            DoCast(target, SPELL_FLAME_SHOCK, false);
                        events.RescheduleEvent(EVENT_FLAME_SHOCK, urand(10000, 12000));
                        break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };
};

class npc_enslaved_rock_elemental: public CreatureScript
{
    public:
        npc_enslaved_rock_elemental() : CreatureScript("npc_enslaved_rock_elemental") {}

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_enslaved_rock_elementalAI(pCreature);
        }

        struct npc_enslaved_rock_elementalAI : public ScriptedAI
        {
            npc_enslaved_rock_elementalAI(Creature *c) : ScriptedAI(c) {}
            
            EventMap events;

            void Reset() override
            {
                events.Reset();
            }

            void EnterCombat(Unit* /*attacker*/) override
            {
                events.RescheduleEvent(EVENT_JAGGED_ROCK_SHIELD, 10000);
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
                    switch(eventId)
                    {
                    case EVENT_JAGGED_ROCK_SHIELD:
                        DoCast(SPELL_JAGGED_ROCK_SHIELD);
                        events.RescheduleEvent(EVENT_JAGGED_ROCK_SHIELD, urand(40000, 45000));
                        break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };
};

class npc_enslaved_gronn_brute: public CreatureScript
{
    public:
        npc_enslaved_gronn_brute() : CreatureScript("npc_enslaved_gronn_brute") {}

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_enslaved_gronn_bruteAI(pCreature);
        }

        struct npc_enslaved_gronn_bruteAI : public ScriptedAI
        {
            npc_enslaved_gronn_bruteAI(Creature* c) : ScriptedAI(c) {}
            
            EventMap events;

            void Reset() override
            {
                events.Reset();
            }

            void EnterCombat(Unit* /*attacker*/) override
            {
                events.RescheduleEvent(EVENT_CRUNCH_ARMOR, urand(2000, 4000));
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while(uint32 eventId = events.ExecuteEvent())
                {
                    switch(eventId)
                    {
                    case EVENT_CRUNCH_ARMOR:
                        if (auto victim = me->getVictim())
                            DoCast(victim, SPELL_CRUNCH_ARMOR, false);
                        events.RescheduleEvent(EVENT_CRUNCH_ARMOR, urand(10000, 15000));
                        break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };
};

class npc_faceless_corruptor : public CreatureScript
{
    public:
        npc_faceless_corruptor() : CreatureScript("npc_faceless_corruptor") {}

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_faceless_corruptorAI(pCreature);
        }

        struct npc_faceless_corruptorAI : public ScriptedAI
        {
            npc_faceless_corruptorAI(Creature* creature) : ScriptedAI(creature)
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

            void Reset() override
            {
                events.Reset();
            }

            void EnterCombat(Unit* /*attacker*/) override
            {
                events.RescheduleEvent(EVENT_SIPHON_ESSENSE, urand(5000, 7000));
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
                    case EVENT_SIPHON_ESSENSE:
                        if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                            DoCast(target, SPELL_SIPHON_ESSENSE, false);
                        events.RescheduleEvent(EVENT_SIPHON_ESSENSE, urand(7000, 10000));
                        break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };
};

class npc_trogg_dweller : public CreatureScript
{
    public:

        npc_trogg_dweller() : CreatureScript("npc_trogg_dweller") {}

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_trogg_dwellerAI(pCreature);
        }

        struct npc_trogg_dwellerAI : public ScriptedAI
        {
            npc_trogg_dwellerAI(Creature *c) : ScriptedAI(c) {}
            
            EventMap events;

            void Reset() override
            {
                events.Reset();
            }

            void EnterCombat(Unit* /*attacker*/) override
            {
                events.RescheduleEvent(EVENT_CLAW_PUNCTURE, urand(5000, 7000));
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
                    switch(eventId)
                    {
                    case EVENT_CLAW_PUNCTURE:
                        if (auto victim = me->getVictim())
                            DoCast(victim, SPELL_CLAW_PUNCTURE, false);
                        events.RescheduleEvent(EVENT_CLAW_PUNCTURE, urand(5000, 7000));
                        break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };
};

class npc_twilight_armsmaster : public CreatureScript
{
    public:
        npc_twilight_armsmaster() : CreatureScript("npc_twilight_armsmaster") {}

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_twilight_armsmasterAI(pCreature);
        }

        struct npc_twilight_armsmasterAI : public ScriptedAI
        {
            npc_twilight_armsmasterAI(Creature* creature) : ScriptedAI(creature) {}
            
            EventMap events;

            void Reset() override
            {
                events.Reset();
            }

            void EnterCombat(Unit* /*attacker*/) override
            {
                events.RescheduleEvent(EVENT_MORTAL_STRIKE, urand(3000, 4000));
                events.RescheduleEvent(EVENT_FLURRY_OF_BLOWS, urand(8000, 10000));
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
                    case EVENT_MORTAL_STRIKE:
                        if (auto victim = me->getVictim())
                            DoCast(victim, SPELL_MORTAL_STRIKE, false);
                        events.RescheduleEvent(EVENT_MORTAL_STRIKE, urand(6000, 8000));
                        break;
                    case EVENT_FLURRY_OF_BLOWS:
                        DoCast(SPELL_FLURRY_OF_BLOWS);
                        events.RescheduleEvent(EVENT_FLURRY_OF_BLOWS, urand(13000, 15000));
                        break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };
};

class npc_twilight_beguiler : public CreatureScript
{
    public:
        npc_twilight_beguiler() : CreatureScript("npc_twilight_beguiler") {}

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_twilight_beguilerAI(pCreature);
        }

        struct npc_twilight_beguilerAI : public ScriptedAI
        {
            npc_twilight_beguilerAI(Creature* creature) : ScriptedAI(creature) {}
            
            EventMap events;

            void Reset() override
            {
                events.Reset();
            }

            void EnterCombat(Unit* /*attacker*/) override
            {
                events.RescheduleEvent(EVENT_CHAINED_MIND, urand(5000, 9000));
                events.RescheduleEvent(EVENT_DECEITFUL_BLAST, urand(2000, 4000));
                
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
                    case EVENT_CHAINED_MIND:
                        if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                            DoCast(target, SPELL_CHAINED_MIND, false);
                        events.RescheduleEvent(EVENT_CHAINED_MIND, urand(8000, 12000));
                        break;
                    case EVENT_DECEITFUL_BLAST:
                        if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                            DoCast(target, SPELL_DECEITFUL_BLAST, false);
                        events.RescheduleEvent(EVENT_DECEITFUL_BLAST, urand(2000, 3000));
                        
                        break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };
};

class npc_twilight_drake : public CreatureScript
{
    public:
        npc_twilight_drake() : CreatureScript("npc_twilight_drake") {}

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_twilight_drakeAI(pCreature);
        }

        struct npc_twilight_drakeAI : public ScriptedAI
        {
            npc_twilight_drakeAI(Creature* creature) : ScriptedAI(creature) {}
            
            EventMap events;

            void Reset() override
            {
                events.Reset();
            }

            void EnterCombat(Unit* /*attacker*/) override
            {
                events.RescheduleEvent(EVENT_TWILIGHT_BREATH, urand(5000, 7000));
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
                    case EVENT_TWILIGHT_BREATH:
                        DoCast(SPELL_TWILIGHT_BREATH);
                        events.RescheduleEvent(EVENT_TWILIGHT_BREATH, urand(10000, 12000));
                        break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };
};

class npc_twilight_earthshaper : public CreatureScript
{
    public:
        npc_twilight_earthshaper() : CreatureScript("npc_twilight_earthshaper") {}

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_twilight_earthshaperAI(pCreature);
        }

        struct npc_twilight_earthshaperAI : public ScriptedAI
        {
            npc_twilight_earthshaperAI(Creature* creature) : ScriptedAI(creature) {}
            
            EventMap events;

            void Reset() override
            {
                DoCast(SPELL_SUMMON_ROCK_ELEMENTAL);
                events.Reset();
            }

            void EnterCombat(Unit* /*attacker*/) override
            {
                events.RescheduleEvent(EVENT_EARTH_SPIKE, urand(3000, 5000));
                events.RescheduleEvent(EVENT_STONE_SKIN, urand(4000, 7000));
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
                    case EVENT_EARTH_SPIKE:
                        if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                            DoCast(target, SPELL_EARTH_SPIKE, false);
                        events.RescheduleEvent(EVENT_EARTH_SPIKE, urand(8000, 10000));
                        break;
                    case EVENT_STONE_SKIN:
                        DoCast(SPELL_STONE_SKIN);
                        events.RescheduleEvent(EVENT_STONE_SKIN, urand(25000, 31000));
                        break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };
};

class npc_twilight_enforcer : public CreatureScript
{
    public:
        npc_twilight_enforcer() : CreatureScript("npc_twilight_enforcer") {}

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_twilight_enforcerAI(pCreature);
        }

        struct npc_twilight_enforcerAI : public ScriptedAI
        {
            npc_twilight_enforcerAI(Creature* creature) : ScriptedAI(creature) {}
            
            EventMap events;

            void Reset() override
            {
                events.Reset();
            }

            void EnterCombat(Unit* /*attacker*/) override
            {
                events.RescheduleEvent(EVENT_MEAT_GRINDER, urand(5000, 6000));
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
                    case EVENT_MEAT_GRINDER:
                        DoCast(SPELL_MEAT_GRINDER_TRIGGER);
                        DoCast(SPELL_MEAT_GRINDER);
                        events.RescheduleEvent(EVENT_MEAT_GRINDER, urand(12000, 15000));
                        break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };
};

class npc_twilight_firecatcher : public CreatureScript
{
    public:
        npc_twilight_firecatcher() : CreatureScript("npc_twilight_firecatcher") {}

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_twilight_firecatcherAI(pCreature);
        }

        struct npc_twilight_firecatcherAI : public ScriptedAI
        {
            npc_twilight_firecatcherAI(Creature* creature) : ScriptedAI(creature) {}
            
            EventMap events;

            void Reset() override
            {
                DoCast(SPELL_SUMMON_BURNING_EMBER);
                events.Reset();
            }

            void EnterCombat(Unit* /*attacker*/) override
            {
                events.RescheduleEvent(EVENT_MOLTEN_BLAST, urand(2000, 3000));
                events.RescheduleEvent(EVENT_FLAME_CONDUIT, urand(6000, 9000));
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
                    case EVENT_MOLTEN_BLAST:
                        if (auto victim = me->getVictim())
                            DoCast(victim, SPELL_MOLTEN_BLAST, false);
                        events.RescheduleEvent(EVENT_MOLTEN_BLAST, urand(8000, 10000));
                        break;
                    case EVENT_FLAME_CONDUIT:
                        if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                            DoCast(target, SPELL_FLAME_CONDUIT, false);
                        events.RescheduleEvent(EVENT_FLAME_CONDUIT, urand(12000, 14000));
                        break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };
};

class npc_twilight_shadow_weaver : public CreatureScript
{
    public:
        npc_twilight_shadow_weaver() : CreatureScript("npc_twilight_shadow_weaver") {}

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_twilight_shadow_weaverAI(pCreature);
        }

        struct npc_twilight_shadow_weaverAI : public ScriptedAI
        {
            npc_twilight_shadow_weaverAI(Creature* creature) : ScriptedAI(creature) {}
            
            EventMap events;

            void Reset() override
            {
                if (IsHeroic())
                    DoCast(SPELL_SUMMON_FELHUNTER);

                events.Reset();
            }

            void EnterCombat(Unit* /*attacker*/)
            {
                events.RescheduleEvent(EVENT_SHADOW_BOLT, urand(1000, 2000));

                if (IsHeroic())
                    events.RescheduleEvent(EVENT_SHADOW_WEAVE, urand(5000, 6000));
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
                    case EVENT_SHADOW_BOLT:
                        if (auto victim = me->getVictim())
                            DoCast(victim, SPELL_SHADOW_BOLT, false);
                        events.RescheduleEvent(EVENT_SHADOW_BOLT, urand(2500, 3000));
                        break;
                    case EVENT_SHADOW_WEAVE:
                        DoCast(SPELL_SHADOW_WEAVE_SCRIPT);
                        events.RescheduleEvent(EVENT_SHADOW_WEAVE, urand(9000, 14000));
                        break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };
};

class npc_twilight_stormbreaker : public CreatureScript
{
    public:
        npc_twilight_stormbreaker() : CreatureScript("npc_twilight_stormbreaker") {}

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_twilight_stormbreakerAI(pCreature);
        }

        struct npc_twilight_stormbreakerAI : public ScriptedAI
        {
            npc_twilight_stormbreakerAI(Creature* creature) : ScriptedAI(creature) {}
            
            EventMap events;

            void Reset() override
            {
                DoCast(SPELL_SUMMON_WATER_SPIRIT);
                events.Reset();
            }

            void EnterCombat(Unit* /*attacker*/) override
            {
                events.RescheduleEvent(EVENT_WATER_BOLT, urand(1000, 2000));

                if (IsHeroic())
                    events.RescheduleEvent(EVENT_WATER_SHELL, urand(5000, 8000));
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
                    case EVENT_WATER_BOLT:
                        if (auto victim = me->getVictim())
                            DoCast(victim, SPELL_WATER_BOLT, false);
                        events.RescheduleEvent(EVENT_WATER_BOLT, urand(2500, 3000));
                        break;
                    case EVENT_WATER_SHELL:
                        DoCast(SPELL_WATER_SHELL);
                        events.RescheduleEvent(EVENT_WATER_SHELL, urand(15000, 20000));
                        break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };
};

class npc_twilight_thundercaller : public CreatureScript
{
    public:
        npc_twilight_thundercaller() : CreatureScript("npc_twilight_thundercaller") {}

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_twilight_thundercallerAI(pCreature);
        }

        struct npc_twilight_thundercallerAI : public ScriptedAI
        {
            npc_twilight_thundercallerAI(Creature* creature) : ScriptedAI(creature) {}
            
            EventMap events;

            void Reset() override
            {
                DoCast(SPELL_SUMMON_THUNDER_SPIRIT);
                events.Reset();
            }

            void EnterCombat(Unit* /*attacker*/) override
            {
                events.RescheduleEvent(EVENT_CHAIN_LIGHTNING, urand(2000, 3000));
                events.RescheduleEvent(EVENT_OVERCHARGE, urand(9000, 12000));
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
                    case EVENT_CHAIN_LIGHTNING:
                        if (auto victim = me->getVictim())
                            DoCast(victim, SPELL_CHAIN_LIGHTNING, false);
                        events.RescheduleEvent(EVENT_CHAIN_LIGHTNING, urand(5000, 7000));
                        break;
                    case EVENT_OVERCHARGE:
                        if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                            DoCast(target, SPELL_OVERCHARGE, false);
                        events.RescheduleEvent(EVENT_OVERCHARGE, urand(10000, 12000));
                        break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };
};

#define SAY_CALL_WYRM "Come to my aid, Drake!"

class npc_twilight_wyrmcaller : public CreatureScript
{
    public:
        npc_twilight_wyrmcaller() : CreatureScript("npc_twilight_wyrmcaller") {}

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_twilight_wyrmcallerAI(pCreature);
        }

        struct npc_twilight_wyrmcallerAI : public ScriptedAI
        {
            npc_twilight_wyrmcallerAI(Creature* creature) : ScriptedAI(creature), summons(me) {}
            
            EventMap events;
            SummonList summons;

            void Reset() override
            {
                summons.DespawnAll();
                events.Reset();
            }

            void EnterCombat(Unit* /*attacker*/) override
            {
                events.RescheduleEvent(EVENT_CALL_WYRM, 3000);
            }

            void JustSummoned(Creature* summon) override
            {
                summons.Summon(summon);
            }
            
            void SummonedCreatureDespawn(Creature* summon) override
            {
                summons.Despawn(summon);
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
                    case EVENT_CALL_WYRM:
                        me->MonsterYell(SAY_CALL_WYRM, 0, ObjectGuid::Empty);
                        if (auto _wyrm = me->SummonCreature(NPC_TWILIGHT_DRAKE_1,
                            me->GetPositionX(),
                            me->GetPositionY(),
                            me->GetPositionZ() + 20.0f,
                            me->GetOrientation()))
                            _wyrm->GetMotionMaster()->MovePoint(0,
                                me->GetPositionX(),
                                me->GetPositionY(),
                                me->GetPositionZ());
                        events.RescheduleEvent(EVENT_FEED_PET, urand(3000, 5000));
                        break;
                    case EVENT_FEED_PET:
                        if (auto _wyrm = me->FindNearestCreature(NPC_TWILIGHT_DRAKE_1, 100.0f, true))
                            DoCast(_wyrm, SPELL_FEED_PET, false);
                        break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };
};

class npc_twilight_war_mage : public CreatureScript
{
    public:
        npc_twilight_war_mage() : CreatureScript("npc_twilight_war_mage") {}

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_twilight_war_mageAI(pCreature);
        }

        struct npc_twilight_war_mageAI : public ScriptedAI
        {
            npc_twilight_war_mageAI(Creature* creature) : ScriptedAI(creature) {}
            
            EventMap events;

            void Reset() override
            {
                events.Reset();
            }

            void EnterCombat(Unit* /*attacker*/) override
            {
                events.RescheduleEvent(EVENT_ENCHANT, 1000);
                events.RescheduleEvent(EVENT_POLYMORPH, urand(5000, 7000));
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
                    case EVENT_ENCHANT:
                        DoCast(RAND(SPELL_FIRE_ENCHANT, SPELL_ICE_ENCHANT));
                        break;
                    case EVENT_POLYMORPH:
                        if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 1, 0.0f, true))
                            DoCast(target, SPELL_POLYMORPH, false);

                        events.RescheduleEvent(EVENT_POLYMORPH, urand(10000, 15000));
                        break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };
};

class npc_ascended_rockbreaker_fissure : public CreatureScript
{
    public:
        npc_ascended_rockbreaker_fissure() : CreatureScript("npc_ascended_rockbreaker_fissure") {}

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_ascended_rockbreaker_fissureAI(pCreature);
        }

        struct npc_ascended_rockbreaker_fissureAI : public Scripted_NoMovementAI
        {
            npc_ascended_rockbreaker_fissureAI(Creature *c) : Scripted_NoMovementAI(c)
            {
                SetCombatMovement(false);
                me->SetReactState(REACT_PASSIVE);
            }

            void Reset() override
            {
                DoCast(SPELL_FISSURE_TRIGGER);
            }
        };
};

class npc_crimsonborne_warlord_empowering_flames : public CreatureScript
{
    public:
        npc_crimsonborne_warlord_empowering_flames() : CreatureScript("npc_crimsonborne_warlord_empowering_flames") {}

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_crimsonborne_warlord_empowering_flamesAI(pCreature);
        }

        struct npc_crimsonborne_warlord_empowering_flamesAI : public Scripted_NoMovementAI
        {
            npc_crimsonborne_warlord_empowering_flamesAI(Creature *c) : Scripted_NoMovementAI(c)
            {
                SetCombatMovement(false);
                me->SetReactState(REACT_PASSIVE);
            }

            void Reset() override
            {
                DoCast(SPELL_EMPOWERING_TWILIGHT_AURA);
            }
        };
};

class spell_twilight_enforcer_meat_grinder : public SpellScriptLoader
{
    public:
        spell_twilight_enforcer_meat_grinder() : SpellScriptLoader("spell_twilight_enforcer_meat_grinder") {}

        class spell_twilight_enforcer_meat_grinder_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_twilight_enforcer_meat_grinder_AuraScript);

            void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (!GetCaster())
                    return;

                GetCaster()->CastSpell(GetCaster(), SPELL_DIZZY, true);
            }
            
            void Register()
            {
                OnEffectRemove += AuraEffectRemoveFn(spell_twilight_enforcer_meat_grinder_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_twilight_enforcer_meat_grinder_AuraScript();
        }
};

class spell_twilight_shadow_weaver_shadow_weave : public SpellScriptLoader
{
    public:
        spell_twilight_shadow_weaver_shadow_weave() : SpellScriptLoader("spell_twilight_shadow_weaver_shadow_weave") {}


        class spell_twilight_shadow_weaver_shadow_weave_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_twilight_shadow_weaver_shadow_weave_SpellScript);


            void HandleScript(SpellEffIndex /*effIndex*/)
            {
                if (!GetCaster())
                    return;

                GetCaster()->CastSpell(GetCaster(), SPELL_SHADOW_WEAVE_DUMMY, true);
                GetCaster()->CastSpell(GetCaster(), SPELL_SHADOW_WEAVE_DMG, false);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_twilight_shadow_weaver_shadow_weave_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_twilight_shadow_weaver_shadow_weave_SpellScript();
        }
};

class spell_twilight_thundercaller_electric_blast: public SpellScriptLoader
{
    public:
        spell_twilight_thundercaller_electric_blast() : SpellScriptLoader("spell_twilight_thundercaller_electric_blast") {}


        class spell_twilight_thundercaller_electric_blast_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_twilight_thundercaller_electric_blast_SpellScript);


            void HandleScript()
            {
                if (!GetCaster())
                    return;

                uint8 _stacks = 0;

                if (GetCaster()->HasAura(SPELL_OVERCHARGE))
                    if (Aura* _overcharge = GetCaster()->GetAura(SPELL_OVERCHARGE))
                        _stacks = _overcharge->GetStackAmount();

                if (_stacks > 0)
                    SetHitDamage(GetHitDamage() * _stacks);
            }

            void Register()
            {
                BeforeHit += SpellHitFn(spell_twilight_thundercaller_electric_blast_SpellScript::HandleScript);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_twilight_thundercaller_electric_blast_SpellScript();
        }
};

void AddSC_grim_batol()
{
    new npc_ascended_flameseeker();
    new npc_ascended_rockbreaker();
    new npc_ascended_waterlasher();
    new npc_ascended_windwalker();
    new npc_azureborne_guardian();
    new npc_azureborne_seer();
    new npc_azureborne_warlord();
    new npc_crimsonborne_guardian();
    new npc_crimsonborne_seer();
    new npc_crimsonborne_warlord();
    new npc_enslaved_burning_ember();
    new npc_enslaved_rock_elemental();
    new npc_enslaved_gronn_brute();
    new npc_faceless_corruptor();
    new npc_trogg_dweller();
    new npc_twilight_armsmaster();
    new npc_twilight_beguiler();
    new npc_twilight_drake();
    new npc_twilight_earthshaper();
    new npc_twilight_enforcer();
    new npc_twilight_firecatcher();
    new npc_twilight_shadow_weaver();
    new npc_twilight_stormbreaker();
    new npc_twilight_thundercaller();
    new npc_twilight_war_mage();
    new npc_twilight_wyrmcaller();
    new npc_ascended_rockbreaker_fissure();
    new npc_crimsonborne_warlord_empowering_flames();
    new spell_twilight_enforcer_meat_grinder();
    new spell_twilight_shadow_weaver_shadow_weave();
    new spell_twilight_thundercaller_electric_blast();
}