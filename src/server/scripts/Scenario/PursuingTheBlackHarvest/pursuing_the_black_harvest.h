/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2006-2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DEF_PURSING_THE_BLACK_HARVEST
#define DEF_PURSING_THE_BLACK_HARVEST

enum Spells
{
    //< Misc
    SPELL_PLACE_EMPOWED_SOULCORE        = 138680, //< SS

    //< S0
    SPELL_WHAT_THE_DRAENEI_FOUND_INTRO  = 151967,
    SPELL_CSA_AT_DUMMY_TIMED_AURA       = 100340,
    SPELL_ENTER_THE_BLACK_TEMPLE        = 134608,
    SPELL_SEARCHING_FOR_INTRUDERS       = 134110, //< AT
    SPELL_DETECTED_PULSE                = 134109,

    //< S1
    SPELL_NETTED                        = 134111,

    //< S2
    SPELL_UPDATE_PLAYER_PHASE_AURAS     = 134209,
    SPELL_BLACKOUT                      = 134112,
    SPELL_SAP                           = 134205,
    SPELL_STEALTH                       = 86603,

    //< S3
    SPELL_TRUSTED_BY_THE_ASHTONGUE      = 134206,
    SPELL_INVISIBILITY_DETECTION        = 49416,

    //< S4
    SPELL_SHADOW_BOLT_2                 = 12739,
    SPELL_SOUL_BLAST                    = 50992,
    SPELL_SUMMON_HUNGERING_SOUL_FRAGMENT= 134207, //< triggered by 134208 from GO 216366
    SPELL_MEMORY_OF_THE_RELIQUARY       = 134210,
    SPELL_SPAWN_THE_RELIQUARY           = 134211,

    //< S5
    SPELL_SPELLFLAME_DUMMY              = 134234,
    SPELL_SPELLFLAME_TRIGGER            = 134235,
    SPELL_HELLFIRE                      = 134225,
    SPELL_HELLFIRE_TRIGGER              = 134224,

    //< S6
    SPELL_SHOOT                         = 41188,
    SPELL_MULTI_SHOOT                   = 41187,
    SPELL_CLEAVE                        = 15284,
    SPELL_SWEEPING_WING_CLIP            = 39584,
    SPELL_SONIC_STRIKE                  = 41168,
    SPELL_SHADOW_BOLT                   = 34344,
    SPELL_DAZED                         = 1604,
    SPELL_FIREBOLT                      = 134245,
    SPELL_LIGHTING_BOLT                 = 42024,
    SPELL_SHADOW_INFERNO2               = 39645,
    SPELL_SUMMON_SHADOWFIENDS           = 39649,

    //< S7
    SPELL_SUMMON_SHADOWFIEND            = 41159,
    SPELL_SHADOW_INFERNO                = 39646,
    SPELL_SMELT_FLASH                   = 37629,
    SPELL_SUMMON_FEL_IMP                = 112866,
    SPELL_INTERRUPTED                   = 134340,
    SPELL_PLUNDER                       = 134323,
    SPELL_UPDATE_PHASE_SHIFT            = 82238,
    SPELL_APPRAISAL                     = 134280,
    
    SPELL_GOLD_RING                     = 134290,
    SPELL_HIGH_ELF_STATUE               = 134294,
    SPELL_FAMILY_JEWELS                 = 134298,
    SPELL_ANCIENT_ORC_SHIELD            = 134302,
    SPELL_EXPENSIVE_RUBY                = 134283,
    SPELL_SPELLSTONE_NECKABLE           = 134287,
    SPELL_SMALL_PILE_OF_GOLD            = 134291,
    SPELL_GOLDON_POTION                 = 134295,
    SPELL_FRUIT_BOWL                    = 134299,
    SPELL_RUNEBLADE                     = 134303,
    SPELL_SPARKLING_SAPPHIRE            = 134284,
    SPELL_DIAMONG_RING                  = 134288,
    SPELL_LARGE_PILE_OF_GOLD            = 134292,
    SPELL_GOLDER_PLATTER                = 134296,
    SPELL_ORNATE_PORTRAIT               = 134300,
    SPELL_FRAGRANT_PERFUME              = 134281,
    SPELL_JADE_KITTEN                   = 134285,
    SPELL_RUBY_RING                     = 134289,
    SPELL_GOLDEN_GOBLET                 = 134293,
    SPELL_YARN                          = 134297,
    SPELL_ROPE_BINDINGS                 = 134301,
    SPELL_CHEAP_COLOGNE                 = 134282,
    SPELL_RUBY_NEACKABLE                = 134286,

    //< Last Step
    // 11859
    SPELL_RITUAL_ENSLAVEMENT            = 22987,  //< SS
    SPELL_DOOMGUARD_SUMMON_DND          = 42010,  //< SS
    SPELL_DOOM_BOLT                     = 85692,  //< SS

    //< NPC_PIT_LORD
    SPELL_FEL_FLAME_BREATH_DUMMY        = 138813,
    SPELL_FEL_FLAME_BREATH              = 138814,
    SPELL_CLEAVE_2                      = 138794,
    SPELL_CHARGE                        = 138796,
    SPELL_CHARGE_TRIGGER                = 138827,
    SPELL_DEMONIC_SIPHON                = 138829,

    // 70023
    SPELL_ANNOUANCE                     = 138629,

    // NPC_WILD_IMP
    SPELL_FEL_FIREBOLT                  = 138747,

    // NPC_JUBEKA_SHADOWBREAKER
    SPELL_ETERNAL_BANISHMENT            = 139186,
    SPELL_FACE_PLAYER                   = 139053, //< SS

    // NPC_KANRETHAD_EBONLOCKE
    SPELL_BURNING_EMBERS                = 138557, //< SS
    SPELL_SOULSHARDS                    = 138556, //< SS
    SPELL_METAMORPHOSIS                 = 138555,
    SPELL_SUMMONING_PIT_LORD            = 138789,
    SPELL_RAID_OF_FIRE                  = 138561,
    SPELL_SEED_OF_TERRIBLE_DESTRUCTION  = 138587,
    SPELL_AURA_OF_OMNIPOTENCE           = 138563,
    SPELL_CURSE_OF_ULTIMATE_DOOM        = 138558,
    SPELL_EXCRUCIATING_AGONY            = 138560,
    SPELL_SOULFIRE                      = 138554,
    SPELL_CHAOS_BOLT                    = 138559,
    SPELL_SEED_OF_TERRIBLE_DESTRUCTION2 = 138588,
    SPELL_BACKFIRE                      = 138619,
    SPELL_CATACLYSM                     = 138564,
    SPELL_ANNOUING_IMP                  = 138621,
    SPELL_SUMMON_WILD_IMPS              = 138685,
    SPELL_ANNIHILATE_DEMONS             = 139141,
    SPELL_DEMONIC_GRASP                 = 139142,
    SPELL_SUMMON_FELHUNTERS             = 138751,
    SPELL_SUMMON_DOOM_LORD              = 138755,

    //< NPC_DOOM_LORD
    SPELL_DOOM_BOLT2                    = 138753,

    //< NPC_FELHUNTER
    SPELL_DEVOUR_ENSLAVEMENT            = 139060,
    SPELL_DEVOUR_MAGIC                  = 139059,
    SPELL_SHADOW_BITE                   = 138750,

    SPELL_DEMONIC_GATEWAY               = 138649, //< SS

    //< Gree fire learning
    SPELL_DRAIN_FEL_ENERGY_DUMMY        = 139200,
    SPELL_FEL_ENERGY_DUMMY              = 140164,
    SPELL_FEL_ENERGY_DUMMY_2            = 140116,
    SPELL_FEL_ENERGY_DUMMY_3            = 140137,
    SPELL_FEL_ENERGY_DUMMY_4            = 140141,
    SPELL_FEL_ENERGY_DUMMY_5            = 140136,
    SPELL_FEL_ENERGY_DUMMY_6            = 140135,
    SPELL_FEL_ENERGY_DUMMY_7            = 140159,
    SPELL_FEL_ENERGY_DUMMY_8            = 140160,
    SPELL_FEL_ENERGY_DUMMY_9            = 140161,
    SPELL_FEL_ENERGY_DUMMY_10           = 140163,
    SPELL_THE_CODEX_OF_XERRATH          = 101508,
    SPELL_THE_CODEX_OF_XERRATH_2        = 137206,

    SPELL_PURGE_XERRATH                 = 139366,
};

enum Stages
{
    STAGE_NONE,

    STAGE_1,
    STAGE_2,
    STAGE_3,
    STAGE_4,
    STAGE_5,
    STAGE_6,
    STAGE_7,
    STAGE_8,
    STAGE_LAST
};

enum Data
{
    DATA_NONE,

    DATA_ALLOWED_STAGE,

    DATA_STAGE_2,
    DATA_ESSENCE_OF_ORDER_EVENT,
    DATA_SCENE_EVENT,
    DATA_PLUNDER_EVENT,
    DATA_AKAMA,
    DATA_NOBEL_EVENT,
    DATA_KANRETHAD,
    DATA_MAIN_DOORS,
    DATA_SECOND_DOOR,
};

enum eCreatures
{
    NPC_AKAMA                       = 68137,
    NPC_ASTHONGUE_PRIMALIST         = 68096,
    NPC_ASHTONGUE_WORKER            = 68098,
    NPC_SUFFERING_SOUL_FRAGMENT     = 68139,
    NPC_HUNGERING_SOUL_FRAGMENT     = 68140,
    NPC_ESSENCE_OF_ORDER            = 68151,
    NPC_LOST_SOULS                  = 68156,
    NPC_UNBOUND_NIGHTLORD           = 68174,
    NPC_UNBOUND_CENTURION           = 68176,
    NPC_UNBOUND_BONEMENDER          = 68175,
    NPC_UNBOUND_SUCCUB              = 68205,
    NPC_FREED_IMP                   = 68173,
    NPC_DEMONIC_SOULWELL            = 70052,
    NPC_KANRETHAD_EBONLOCKE         = 69964,
    NPC_DEMONIC_GATEWAY             = 70028,
    NPC_JUBEKA_SHADOWBREAKER        = 70166,
    NPC_WILD_IMP                    = 70071,
    NPC_PIT_LORD                    = 70075,
    NPC_PORTALS_VISUAL              = 24925,
    NPC_FEL_IMP                     = 58959,
    NPC_ASHTONGUE_SHAMAN            = 68129,
    NPC_DOOM_LORD                   = 70073,
    NPC_FELHUNTER                   = 70072,
};

enum eGameObects
{
    GO_CONSPICUOUS_ILLIDARI_SCROLL  = 216364,
    GO_MAIN_TEMPLATE_DOORS          = 185882,
    GO_SECOND_DOOR                  = 185479,
    GO_TRAP                         = 216366,

    GO_TREASURE_CHEST               = 216452, //< quest ender

    //< Gobs for loot...
    GO_GOLDEN_HIGH_ELF_STATUETTE    = 216442,
    GO_GOLD_PLATTER                 = 216444,
    GO_SHINY_YARN                   = 216445,
    GO_GORGEOUS_GEM                 = 216446,
    GO_GOLD_FRUIT_BOWL              = 216447,
    GO_DUSTY_PAINTING               = 216448,
    GO_FLUFFY_PILLOW                = 216449,
    GO_ANCIENT_ORCISH_SHIELD        = 216450,
    GO_RUSTED_SWORD                 = 216451,
    GO_FRAGRANT_PERFUME             = 216428,
    GO_COLOGNE                      = 216430,
    GO_EXPENSIVE_RUBY               = 216431,
    GO_SPARKLING_SAPPHIRE           = 216432,
    GO_JADE_KITTEN                  = 216433,
    GO_RUBY_NECKLACE                = 216434,
    GO_SPELLSTONE_NECKLACE          = 216435,
    GO_DIAMOND_RING                 = 216436,
    GO_RUBY_RING                    = 216437,
    GO_GOLD_RING                    = 216438,
    GO_SMALL_PILE_OF_COINS          = 216439,
    GO_LARGE_PILE_OF_COINS          = 216440,
    GO_GOLDEN_GOBLET                = 216441,
};

bool IsNextStageAllowed(InstanceScript* instance, uint8 stage);

#endif
