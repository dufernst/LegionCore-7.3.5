/*
    Dungeon : Mogushan Palace 88-90
    Instance General Script
*/

#ifndef MOGUSHAN_VAULT_H_
#define MOGUSHAN_VAULT_H_

enum eData
{
    DATA_STONE_GUARD                = 0,
    DATA_FENG                       = 1,
    DATA_GARAJAL                    = 2,
    DATA_SPIRIT_KINGS               = 3,
    DATA_ELEGON                     = 4,
    DATA_WILL_OF_EMPEROR            = 5,
    DATA_MAX_BOSS_DATA              = 6
};

enum eActions
{
    ACTION_ENTER_COMBAT             = 1,
    ACTION_FAIL                     = 2,

    // Stone Guard
    ACTION_GUARDIAN_DIED            = 3,
    ACTION_PETRIFICATION            = 4,

    // Feng
    ACTION_SPARK                    = 5,

    // Spirit Kings
    ACTION_START_FIGHT              = 6,
    ACTION_FLANKING_MOGU            = 7,
    ACTION_SPIRIT_LOW_HEALTH        = 8,
    ACTION_SPIRIT_KILLED            = 9,
    ACTION_SPIRIT_DONE              = 10 
};

enum eCreatures
{
    // The Stone Guard
    NPC_STONE_GUARD_CONTROLER       = 60089,
    NPC_JASPER                      = 59915,
    NPC_JADE                        = 60043,
    NPC_AMETHYST                    = 60047,
    NPC_COBALT                      = 60051,

    // Feng
    NPC_FENG                        = 60009,
    NPC_PHASE_CONTROLER             = 61124,

    // Garajal
    NPC_GARAJAL                     = 60143,
    NPC_SHADOWY_MINION_REAL         = 60940,
    NPC_SHADOWY_MINION_SPIRIT       = 60184,
    NPC_SOUL_CUTTER                 = 62003,
    NPC_SPIRIT_TOTEM                = 60240,

    // Spirit kings
    NPC_SPIRIT_GUID_CONTROLER       = 60984,

    NPC_ZIAN                        = 60701,
    NPC_MENG                        = 60708,
    NPC_QIANG                       = 60709,
    NPC_SUBETAI                     = 60710,

    NPC_FLANKING_MOGU               = 60847,
    NPC_PINNING_ARROW               = 60958,
    NPC_UNDYING_SHADOW              = 60731,
    NPC_VOLLEY                      = 60942,

    // Elegon
    NPC_ELEGON                      = 60410,
    NPC_INVISIBLE_STALKER           = 65297,
    NPC_ENERGY_VORTEX_STALKER       = 61330,
    NPC_COSMIC_SPARK                = 62618,
    NPC_ENERGY_CHARGE               = 60913,
    NPC_EMPYREAL_FOCUS              = 60776,

    // Woi Controller
    NPC_WOI_CONTROLLER              = 200467, //New trigger
    
    // Will of Emperor
    NPC_QIN_XI                      = 60399,
    NPC_JAN_XI                      = 60400,

    // Imperator adds
    NPC_RAGE                        = 60396,
    NPC_FORCE                       = 60397,
    NPC_COURAGE                     = 60398,
};

enum eGameObjects
{
    GOB_STONE_GUARD_DOOR_ENTRANCE   = 214497,
    GOB_STONE_GUARD_DOOR_EXIT       = 214526,
    GOB_FENG_DOOR_FENCE             = 214452, // Both entrance and exit
    GOB_FENG_DOOR_EXIT              = 214696,
    GOB_GARAJAL_FENCE               = 213933, // Both entrance and exit
    GOB_GARAJAL_EXIT                = 213285,
    GOB_SPIRIT_KINGS_WIND_WALL      = 215114,
    GOB_SPIRIT_KINGS_EXIT           = 213373,
    GOB_ELEGON_DOOR_ENTRANCE        = 213244,
    GOB_ELEGON_CELESTIAL_DOOR       = 214412,
    GOB_WILL_OF_EMPEROR_ENTRANCE    = 213258,

    // Feng
    GOB_SPEAR_STATUE                = 213245,
    GOB_FIST_STATUE                 = 213246,
    GOB_SHIELD_STATUE               = 213247,
    GOB_STAFF_STATUE                = 213248,
    GOB_INVERSION                   = 211628,
    GOB_CANCEL                      = 211626,

    // Elegon
    GOB_ENERGY_PLATFORM             = 213526,
    GOB_ENERGY_TITAN_DISK           = 213506,
    GOB_ENERGY_TITAN_CIRCLE_1       = 213528,
    GOB_ENERGY_TITAN_CIRCLE_2       = 213527,
    GOB_ENERGY_TITAN_CIRCLE_3       = 213529,

    GOB_MOGU_RUNE_FIRST             = 213937,
    GOB_MOGU_RUNE_END               = 213955,
};

#endif // MOGUSHAN_VAULT_H_
