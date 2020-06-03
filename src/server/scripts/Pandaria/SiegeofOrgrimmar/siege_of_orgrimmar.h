
//Siege of Orgrimmar

#ifndef SIEGEOFORGRIMMAR
#define SIEGEOFORGRIMMAR

#include "SpellScript.h"
#include "Map.h"
#include "Creature.h"
#include "CreatureAIImpl.h"

enum globalSpells
{
    SPELL_SHA_VORTEX                = 146024,
    SPELL_TP_ORGRIMMAR_1            = 148032, //Horde
    SPELL_TP_ORGRIMMAR_2            = 148034, //Aliance
};

enum ssActions
{
    ACTION_RESET                    = 255,
    ACTION_IN_PROGRESS              = 256,
    ACTION_SSOPS_IN_PROGRESS        = 257,
    ACTION_SSOPS_DONE               = 258,
    ACTION_SSOPS_SECOND_ROOM        = 259,
    ACTION_KLAXXI_IN_PROGRESS       = 260,
    ACTION_KLAXXI_DONE              = 270,
    ACTION_LAUNCH_ANNIHILLATE       = 271,
    ACTION_FIRST_ENGENEER_DIED      = 272,
    ACTION_CANCEL_ANNIHILLATE       = 273,
    ACTION_END_DESPERATE_MEASURES   = 274,
};

enum eData
{
    // Encounter States/Boss GUIDs
    DATA_IMMERSEUS                  = 0,
    DATA_F_PROTECTORS               = 1,
    DATA_NORUSHEN                   = 2,
    DATA_SHA_OF_PRIDE               = 3,
    DATA_GALAKRAS                   = 4,
    DATA_IRON_JUGGERNAUT            = 5,
    DATA_KORKRON_D_SHAMAN           = 6,
    DATA_GENERAL_NAZGRIM            = 7,
    DATA_MALKOROK                   = 8,
    DATA_SPOILS_OF_PANDARIA         = 9,
    DATA_THOK                       = 10,
    DATA_BLACKFUSE                  = 11,
    DATA_KLAXXI                     = 12,
    DATA_GARROSH                    = 13,
    DATA_MAX,

    DATA_FIELD_OF_SHA,
    DATA_FP_EVADE,
    DATA_SHA_PRE_EVENT,
    DATA_SOP_START,
    DATA_KLAXXI_START,
    DATA_BUFF_NEXT_KLAXXI,
    DATA_IS_KLAXXI_DONE,
    DATA_INTRO_NEXT_KLAXXI,
    DATA_CLEAR_KLAXXI_LIST,
    DATA_SAFE_WEAPONS,
    DATA_D_WEAPON_IN_DEST_POINT,
    DATA_CRAWLER_MINE_READY,
    DATA_CRAWLER_MINE_ENTERCOMBAT,
    DATA_GET_WEAPON_WAVE_INDEX,
    DATA_SHOCK_WAVE_MISSILE,
    DATA_CHECK_INSTANCE_PROGRESS,
    DATA_OPEN_SOLDIER_FENCH,
    DATA_GET_REALM_OF_YSHAARJ,
    DATA_GARROSH_REALM,
    DATA_UPDATE_GARROSH_REALM,
    DATA_FIRST_ENGENEER_DIED,
    DATA_ACTION_SOLDIER,
    DATA_RESET_REALM_OF_YSHAARJ,
    DATA_PLAY_FINAL_MOVIE,
    DATA_CHECK_DIED_PLAYER_IN_REALM_OF_YSHARRJ,
    DATA_CORRUPTED_BREW_COUNT,
    DATA_CALAMITY_COUNT,
    DATA_SAWBLADE_IN_POINT_ELECTROMAGNET,
    DATA_SAWBLADE_IN_POINT_CONVEYER,
    DATA_SAWBLADE_CHANGE_POLARITY_FORWARD,
    DATA_SAWBLADE_CHANGE_POLARITY_BACK,
    DATA_ACTIVE_SUPERHEAT,
    DATA_DESPAWN,
    DATA_GET_FLASH_TARGET,
    DATA_GET_THOK_PHASE_COUNT,
    DATA_CLOSE_ZONE_NORUSHEN,

    //Galakras
    DATA_GALAKRAS_PRE_EVENT,
    DATA_GALAKRAS_PRE_EVENT_COUNT,
    DATA_JAINA_OR_SYLVANA,
    DATA_VEREESA_OR_AETHAS,
    DATA_SOUTH_TOWER,
    DATA_NORTH_TOWER,
    DATA_SOUTH_COUNT,
    DATA_NORTH_COUNT,
    DATA_DEMOLITIONS_EXPERT_S,
    DATA_DEMOLITIONS_EXPERT_N,
    DATA_ACTIVE_NORTH_ROPE,
    DATA_ACTIVE_SOUTH_ROPE,
    DATA_DISABLE_ROPES,
    DATA_START_ROTATE,
    DATA_PREPARE_REALM_OF_YSHAARJ,
    DATA_GARROSH_STORMWIND,
    DATA_KILL_PLAYERS_IN_MIND_CONTROL,

    DATA_TEAM_IN_INSTANCE,

    // Additional data
    DATA_IMMERSEUS_INTRO,
    DATA_SPOIL_MANTIS,
    DATA_SPOIL_MOGU,
    DATA_CHECK_KDS_RESET_IS_DONE,
};

enum eCreatures
{
    //Npc or summons
    NPC_LOREWALKER_CHO              = 73330,
    NPC_LOREWALKER_CHO2             = 72872,
    NPC_LOREWALKER_CHO3             = 61348,
    NPC_CONTAMINATED_PUDDLE         = 71604,
    NPC_AQUEOUS_DEFENDER            = 73191,
    NPC_PUDDLE_POINT                = 90000,
    NPC_SHA_POOL                    = 71544,
    NPC_SHA_PUDDLE                  = 71603,
    NPC_B_H_CONTROLLER              = 90008,
    NPC_BLIND_HATRED                = 72565,
    NPC_BLIND_HATRED_BASE           = 72595,
    NPC_CRAWLER_MINE                = 72050,
    NPC_CUTTER_LASER                = 72026,
    NPC_EXPLOSIVE_TAR               = 71950,
    NPC_BORER_DRILL                 = 71906,
    NPC_ASH_ELEMENTAL               = 71827,
    NPC_IRON_TOMB                   = 71941,
    NPC_KORKRON_BANNER              = 71626,
    NPC_AFTER_SHOCK                 = 71697,
    NPC_HEALING_TIDE_TOTEM          = 72563,
    NPC_ARCING_SMASH                = 71455,
    NPC_ANCIENT_MIASMA              = 71513,
    NPC_IMPLOSION                   = 71470,
    NPC_ESSENCE_OF_YSHAARJ          = 63420,
    NPC_LIVING_CORRUPTION           = 71644,
    NPC_BODY_STALKER                = 71787, //in center room
    NPC_SHOCK_COLLAR                = 71645, //dest dummy
    NPC_STARVED_YETI                = 73184,
    NPC_CAPTIVE_CAVE_BAT            = 73522,
    NPC_SWIRL_TARGET                = 71550,
    NPC_FLASH_STALKER               = 90003,
    NPC_CONGEALED_SHA               = 71642,
    NPC_SWIRL_TRIGGER               = 71548,

    NPC_IMMERSEUS                   = 71543,

    //Fallen Protectors
    NPC_ROOK_STONETOE               = 71475,
    NPC_SUN_TENDERHEART             = 71480,
    NPC_HE_SOFTFOOT                 = 71479,
    NPC_GOLD_LOTOS_MAIN             = 71683,
    NPC_GOLD_LOTOS_MOVER            = 71695,
    NPC_GOLD_LOTOS_HE               = 71711,
    NPC_GOLD_LOTOS_SUN              = 71684,
    NPC_GOLD_LOTOS_ROOK             = 71686,

    NPC_EMBODIED_ANGUISH_OF_HE      = 71478,
    NPC_EMBODIED_DESPERATION_OF_SUN = 71482,
    NPC_EMBODIED_DESPIRE_OF_SUN     = 71474,
    NPC_EMBODIED_MISERY_OF_ROOK     = 71476,
    NPC_EMBODIED_GLOOM_OF_ROOK      = 71477,
    NPC_EMBODIED_SORROW_OF_ROOK     = 71481,
    NPC_DESPAIR_SPAWN               = 71712,
    BPC_DESPERATION_SPAWN           = 71993,

    //Field of Sha
    NPC_ZEAL                        = 72661, //Zeal
    NPC_ARROGANCE                   = 72663, //Arrogance
    NPC_VANITY                      = 72662, //Vanity

    //Sha of Pride
    NPC_SHA_OF_PRIDE                = 71734,
    NPC_LADY_JAINA_PROUDMORORE      = 73598, //Lady Jaina Proudmoore
    NPC_SHA_TARAN_ZHU               = 72779, //Taran Zhu <Lord of the Shado-Pan>
    NPC_SHA_NORUSHEN                = 71965,
    NPC_REFLECTION                  = 72172, //Reflection
    NPC_LINGERING_CORRUPTION        = 72791, //Lingering Corruption
    NPC_MANIFEST_OF_PRIDE           = 71946, //Manifestation of Pride
    NPC_SLG_GENERIC_MOP             = 68553, //SLG Generic MoP
    NPC_RIFT_OF_CORRUPTION          = 72846, //Rift of Corruption
    NPC_SHA_OF_PRIDE_END_LADY_JAINA = 73598,
    NPC_SHA_OF_PRIDE_END_THERON     = 73605,
    NPC_PORTAL_TO_ORGRIMMAR         = 73536,
    //controller 68553?? Sha Vortex triger 146034

    //Norushen
    NPC_NORUSHEN                        = 71967,
    NPC_AMALGAM_OF_CORRUPTION           = 72276,
    NPC_ESSENCE_OF_CORRUPTION_C         = 71976,
    NPC_ESSENCE_OF_CORRUPTION           = 72263, //released
    NPC_MANIFESTATION_OF_CORRUPTION_C   = 71977,
    NPC_MANIFESTATION_OF_CORRUPTION     = 72264, //FRAYED & release
    NPC_TITANIC_CORRUPTION              = 72051,
    NPC_GREATER_CORRUPTION              = 72001,
    NPC_PURIFYING_LIGHT                 = 72065,
    NPC_RESIDUAL_CORRUPTION             = 72550, // by spell 145522
    NPC_NN_HEAL_EVENT_PROTECTOR_1       = 71996,//Rook(meleedmg)
    NPC_NN_HEAL_EVENT_PROTECTOR_2       = 72000,//Sun(caster)
    NPC_NN_HEAL_EVENT_PROTECTOR_3       = 71995,//Leven(tank)

    //Galakras
    NPC_GALAKRAS                    = 72249,
    NPC_WARLORD_ZAELA               = 72248,

    NPC_LADY_JAINA_PROUDMOORE_A     = 72302,
    NPC_LADY_SYLVANAS_WINDRUNNER_H  = 72561,
    NPC_KING_VARIAN_WRYNN_A         = 72311,
    NPC_LORTHEMAR_THERON_H          = 72560,
    NPC_VEREESA_WINDRUNNER_A        = 73910,
    NPC_ARCHMAGE_AETHAS_SUNREAVER_H = 73909,

    NPC_HIGHGUARD_KAILUS_A          = 72312,
    NPC_HIGHGUARD_TYRIUS_A          = 72313,
    NPC_ALLIANCE_VANGUARD_GNOM_A    = 72549,
    NPC_ALLIANCE_VANGUARD_HUMAN_A   = 72548,
    NPC_ALLIANCE_VANGUARD_DWARF_A   = 72534,
    NPC_SUNREAVER_MAGUS_H           = 72581,
    NPC_SUNREAVER_SENTINEL_H        = 72579,
    NPC_SUNREAVER_CONSTRUCT_H       = 72580,
    NPC_DEMOLITIONS_EXPERT_S_A      = 73493,
    NPC_DEMOLITIONS_ASSISTANT_S_A   = 73494,
    NPC_DEMOLITIONS_EXPERT_N_A      = 73495,
    NPC_DEMOLITIONS_ASSISTANT_N_A   = 73496,
    NPC_DEMOLITIONS_EXPERT_S_H      = 73550,
    NPC_DEMOLITIONS_ASSISTANT_S_H   = 73551,
    NPC_DEMOLITIONS_EXPERT_N_H      = 73552,
    NPC_DEMOLITIONS_ASSISTANT_N_H   = 73553,

    NPC_TOWER_SOUTH                 = 73565,
    NPC_TOWER_NORTH                 = 73628,
    NPC_ANTIAIR_TURRET              = 72408,

    NPC_MASTER_CANNONEER_DAGRYN     = 72356,
    NPC_LIEUTENANT_KRUGRUK          = 72357,
    NPC_KORKRON_DEMOLISHER          = 72947,
    NPC_HIGH_ENFORCER_THRANOK       = 72355,
    NPC_KORGRA_THE_SNAKE            = 72456,
    NPC_DRAGONMAW_GRUNT             = 72941,
    NPC_DRAGONMAW_GRUNT_H           = 73530,
    NPC_DRAGONMAW_FLAGBEARER        = 72942,
    NPC_DRAGONMAW_PROTO_DRAKE       = 72943,
    NPC_DRAGONMAW_BONECRUSHER       = 72945,
    NPC_DRAGONMAW_EBON_STALKER      = 72352,
    NPC_DRAGONMAW_FLAMESLINGER      = 72353,
    NPC_DRAGONMAW_TIDAL_SHAMAN      = 72958,
    NPC_KORKRON_CANNON              = 72358,
    NPC_SPIKE_MINE                  = 72656,
    //

    NPC_IRON_JUGGERNAUT             = 71466,
    NPC_MORTAR_BARRAGE              = 90002, //new trigger, need update in DB
    //Iron Juggernaut passenger              //Seat Id
    NPC_TOP_CANNON                  = 71484, //0
    NPC_SAWBLADE                    = 71469, //1
    NPC_CANNON                      = 71468, //2, 3
    NPC_TAIL_GUN                    = 71914, //4
    //

    //Korkron Dark Shamans
    NPC_WAVEBINDER_KARDRIS          = 71858,
    NPC_EARTHBREAKER_HAROMM         = 71859,
    //Wolf Mounts
    NPC_BLOODCLAW                   = 71923,
    NPC_DARKFANG                    = 71921,

    NPC_FALLING_ASH                 = 71789,
    NPC_FALLING_ASH_GROUND_STALKER  = 90007,//new trigger, need update in DB
    //

    NPC_GENERAL_NAZGRIM             = 71515,
    //His summons
    NPC_IRONBLADE                   = 71770,
    NPC_ARCHWEAVER                  = 71771,
    NPC_ASSASIN                     = 71518,
    NPC_WARSHAMAN                   = 71773,
    NPC_SNIPER                      = 71656,
    //
    NPC_WILDHAMMER_SHAMAN           = 72640,
    NPC_RESISTANCE_TOTEM            = 72644,

    NPC_MALKOROK                    = 71454,

    //Spoils of Pandaria
    NPC_SSOP_SPOILS                 = 71889, //Secured Stockpile of Pandaren Spoils

    //Spoils 
    NPC_MOGU_SPOILS                 = 73720,
    NPC_MOGU_SPOILS2                = 73722,
    NPC_MANTIS_SPOILS               = 71512,
    NPC_MANTIS_SPOILS2              = 73721,
    
    //Big 
    //Mogu
    NPC_JUN_WEI                     = 73723,
    NPC_ZU_YIN                      = 73724,
    NPC_XIANG_LIN                   = 73725,
    NPC_KUN_DA                      = 71408,
    //Mantis
    NPC_COMMANDER_ZAKTAR            = 71409,
    NPC_COMMANDER_IKTAL             = 73948,
    NPC_COMMANDER_NAKAZ             = 73949,
    NPC_COMMANDER_TIK               = 73951,
    //

    //Medium 
    //Mogu
    NPC_MODIFIED_ANIMA_GOLEM        = 71395,
    NPC_MOGU_SHADOW_RITUALIST       = 71393,
    NPC_SHADOW_RITUALIST_PHYLACTERY = 71392,
    //Mantis
    NPC_ZARTHIK_AMBER_PRIEST        = 71397,
    NPC_SETTHIK_WIND_WIELDER        = 71405,
    //

    //Small 
    //Mogu
    NPC_ANIMATED_STONE_MOGU         = 71380,
    NPC_BURIAL_URN                  = 71382,
    NPC_QUILEN_GUARDIANS            = 71378,
    NPC_SPARK_OF_LIFE               = 71433,
    //Mantis
    NPC_SRITHIK_BOMBARDIER          = 71385,
    NPC_AMBER_ENCASED_KUNCHONG      = 71388,
    NPC_KORTHIK_WARCALLER           = 71383,
    //
    NPC_STONE_STATUE                = 72535,

    //Summons
    NPC_ZARTHIK_SWARMER             = 71398,

    //Pandaren Relic box                     //display id and name from - entry, needed update in DB
    NPC_NAMELESS_WINDWALKER_SPIRIT  = 72828, //dd       71430
    NPC_WISE_MISTWEAVER_SPIRIT      = 72810, //Healers  71428
    NPC_ANCIENT_BREWMASTER_SPIRIT   = 72787, //Tank     71427
    //

    //Special
    NPC_LEVER                       = 72281,
    NPC_LIFT_HOOK                   = 72972,
    NPC_UNSTABLE_SPARK              = 73104,
    //

    NPC_THOK                        = 71529,
    //Jailer
    NPC_KORKRON_JAILER              = 71658,
    //Prisoners
    NPC_AKOLIK                      = 71742, //GO_SAUROK_JAIL
    NPC_MONTAK                      = 71763, //GO_YAUNGOLIAN_JAIL
    NPC_WATERSPEAKER_GORAI          = 71749, //GO_JINUI_JAIL

    //Siegecrafter Blackfuse                 //Seat Id
    NPC_BLACKFUSE                   = 72694, //1
    NPC_BLACKFUSE_MAUNT             = 71504,
    NPC_BLACKFUSE_SAWBLADE          = 71532, //0
    //Triggers
    NPC_LASER_ARRAY                 = 71910,
    NPC_LASER_TARGET                = 71740,
    NPC_SHOCKWAVE_MISSILE_STALKER   = 72710,
    NPC_SHOCKWAVE_MISSILE           = 72052,
    NPC_SIEGE_ENGINEER_HELPER       = 71520,
    //Summons
    NPC_AUTOMATED_SHREDDER          = 71591,
    //Left Counveyer(offline)
    NPC_DISASSEMBLED_CRAWLER_MINE   = 71790,
    NPC_DEACTIVATED_LASER_TURRET    = 71751,
    NPC_DEACTIVATED_ELECTROMAGNET   = 71694,
    NPC_DEACTIVATED_MISSILE_TURRET  = 71606,
    //Right Counveyer(online)
    NPC_BLACKFUSE_CRAWLER_MINE      = 71788,
    NPC_ACTIVATED_LASER_TURRET      = 71752,
    NPC_ACTIVATED_ELECTROMAGNET     = 71696,
    NPC_ACTIVATED_MISSILE_TURRET    = 71638,
    //
    NPC_OVERCHARGED_ELECTROMAGNET   = 73387,

    //Paragons of the Klaxxi                 
    NPC_KILRUK                      = 71161, //6
    NPC_XARIL                       = 71157, //4
    NPC_KAZTIK                      = 71156, //5
    NPC_KORVEN                      = 71155, //2
    NPC_IYYOKYK                     = 71160, //3
    NPC_KAROZ                       = 71154, //1 
    NPC_SKEER                       = 71152,
    NPC_RIKKAL                      = 71158,
    NPC_HISEK                       = 71153,

    NPC_BLOOD                       = 71542,
    NPC_AMBER                       = 71407,
    NPC_AMBER_PIECE                 = 71628,
    NPC_KLAXXI_CONTROLLER           = 71592,
    NPC_AMBER_PARASITE              = 71578,
    NPC_AMBER_BOMB                  = 71691,
    NPC_HUNGRY_KUNCHONG             = 71420,
    NPC_EERIE_FOG                   = 71453, //stalker
    //

    NPC_GARROSH                     = 71865,
    //Garrosh soldiers
    NPC_SIEGE_ENGINEER              = 71984,
    NPC_WARBRINGER                  = 71979,
    NPC_WOLF_RIDER                  = 71983,
    //
    NPC_MINION_OF_YSHAARJ           = 72272,
    NPC_DESECRATED_WEAPON           = 72154,
    NPC_EMPOWERED_DESECRATED_WEAPON = 72198,
    NPC_HEART_OF_YSHAARJ            = 72215,
    NPC_HEART_OF_YSHAARJ_REALM      = 72228,
    NPC_KORKRON_IRON_STAR           = 71985,
    NPC_KORKRON_IRON_STAR_HM        = 73059,
    NPC_THRALL                      = 73483,
    NPC_THE_LAST_GASP_OF_YSHAARJ    = 73335,
    //Adds in Tempels
    NPC_SHA_VORTEX                  = 72239,
    //Temple of the Red Crane
    NPC_EMBODIED_DESPAIR            = 72238,
    //Temple of the Jade Serpent
    NPC_EMBODIED_DOUBT              = 72236,
    //Terrace of Endless Spring
    NPC_EMBODIED_FEAR               = 72237,
    //HM, Last Phase
    NPC_MANIFESTATION_OF_RAGE       = 73253,
    NPC_KORKRON_GUNSHIP             = 73065,
    NPC_HORDE_CANNON                = 73665,
    NPC_CLUMP                       = 73278,
    NPC_PORTAL_TO_REALITY           = 74007,
};

enum eGameObjects
{   
    GO_SHIP_ALLIANCE                = 223459,
    GO_SHIP_HORDE                   = 223464,

    //Immerseus
    GO_IMMERSEUS_EX_DOOR            = 221663,

    //Fallen Protectors
    GO_SHA_FIELD                    = 221611,

    //Field of Sha
    GO_SHA_ENERGY_WALL              = 221602,

    //Sha
    GO_VAULT_OF_FORBIDDEN_TREASURES_1 = 221739, //Vault of Forbidden Treasures
    GO_VAULT_OF_FORBIDDEN_TREASURES_2 = 221741,
    GO_VAULT_OF_FORBIDDEN_TREASURES_3 = 221742, //25Heroic
    GO_VAULT_OF_FORBIDDEN_TREASURES_4 = 221740,
    GO_VAULT_OF_FORBIDDEN_TREASURES_5 = 233029, //Mythic
    GO_VAULT_OF_FORBIDDEN_TREASURES_6 = 232163,
    GO_VAULT_OF_FORBIDDEN_TREASURES_7 = 232164,
    
    GO_CORRUPTED_PRISON_WEST        = 221678,
    GO_CORRUPTED_BUTTON_WEST_1      = 221753,
    GO_CORRUPTED_BUTTON_WEST_2      = 221751,
    GO_CORRUPTED_BUTTON_WEST_3      = 221752,
    GO_CORRUPTED_PRISON_EAST        = 221676,
    GO_CORRUPTED_BUTTON_EAST_1      = 221758,
    GO_CORRUPTED_BUTTON_EAST_2      = 221759,
    GO_CORRUPTED_BUTTON_EAST_3      = 221757,
    GO_CORRUPTED_PRISON_NORTH       = 221677,
    GO_CORRUPTED_BUTTON_NORTH_1     = 221755,
    GO_CORRUPTED_BUTTON_NORTH_2     = 221750,
    GO_CORRUPTED_BUTTON_NORTH_3     = 221754,
    GO_CORRUPTED_PRISON_SOUTH       = 221679,
    GO_CORRUPTED_BUTTON_SOUTH_1     = 221761,
    GO_CORRUPTED_BUTTON_SOUTH_2     = 221760,
    GO_CORRUPTED_BUTTON_SOUTH_3     = 221756,

    //Norushen
    GO_LIGTH_QUARANTINE             = 223142,
    GO_LIGTH_QUARANTINE_2           = 223143,
    GO_LIGTH_QUARANTINE_3           = 223144,
    GO_LIGTH_QUARANTINE_4           = 223145,
    GO_LIGTH_QUARANTINE_5           = 223146,
    GO_LIGTH_QUARANTINE_6           = 223147,
    GO_LIGHT_RAY_01                 = 223192,
    GO_LIGHT_RAY_02                 = 223020,
    GO_LIGHT_RAY_03                 = 223019,
    GO_LIGHT_RAY_04                 = 223018,
    GO_LIGHT_RAY_05                 = 223017,
    GO_LIGHT_RAY_06                 = 223016,
    GO_LIGHT_RAY_07                 = 223015,
    GO_LIGHT_RAY_08                 = 223014,
    GO_LIGHT_RAY_09                 = 223013,
    GO_LIGHT_RAY_10                 = 223012,
    GO_LIGHT_RAY_11                 = 223011,
    GO_LIGHT_RAY_12                 = 223010,
    GO_LIGHT_RAY_13                 = 223009,
    GO_LIGHT_RAY_14                 = 223008,
    GO_LIGHT_RAY_15                 = 223007,
    GO_LIGHT_RAY_16                 = 223021,

    GO_NORUSHEN_EX_DOOR             = 221447,

    //Sha of Pride
    GO_SHA_OF_PRIDE_ENT_DOOR        = 221446,
    
    //Galakras
    GO_SOUTH_DOOR                   = 221916,
    GO_SOUTH_ROPE_SKEIN             = 223281,

    GO_NORTH_DOOR                   = 223044,
    GO_NORTH_ROPE_SKEIN             = 223287,

    GO_ROPE                         = 223282,

    //Korkron Dark Shaman
    GO_KORKRON_CAGE                 = 221564,

    //Iron Juggernaut
    GO_WIND_DOOR                    = 223219,
    GO_ORGRIMMAR_GATE               = 223739,
    GO_ORGRIMMAR_GATE2              = 223814,

    //Blackfuse
    GO_BLACKFUSE_ENT_DOOR           = 223057,

    GO_RUSTY_BARS                   = 223231,
    GO_NAZGRIM_DOOR                 = 223276,
    GO_NAZGRIM_EX_DOOR              = 221793,
    GO_MALKOROK_FENCH               = 221784,
    GO_MALKOROK_FENCH_2             = 221785,

    //Spoils of Pandaria
    GO_SSOP_SPOILS                  = 220823, 
    GO_NSOP_SPOILS                  = 222749,
    GO_ENT_GATE                     = 223056,
    GO_ENT_DOOR_LEFT                = 221800,
    GO_ENT_DOOR_RIGHT               = 221801,
    GO_EX_DOOR_RIGHT                = 221798,
    GO_EX_DOOR_LEFT                 = 221799,

    //Entrance to room
    GO_ROOM_GATE                    = 221794, //NPC_MOGU_SPOILS
    GO_ROOM_GATE2                   = 221795, //NPC_MOGU_SPOILS2, pull
    GO_ROOM_GATE3                   = 221796, //NPC_MANTIS_SPOILS
    GO_ROOM_GATE4                   = 221797, //NPC_MANTIS_SPOILS2, pull

    //Right
    GO_IRON_DOOR_R                  = 223032, 
    GO_LEVER_R                      = 221771,  

    //Left
    GO_IRON_DOOR_L                  = 223033,
    GO_LEVER_L                      = 221773,

    //Boxes
    //Mogu
    GO_SMALL_MOGU_BOX               = 221906,
    GO_MEDIUM_MOGU_BOX              = 221893,
    GO_BIG_MOGU_BOX                 = 221885,
    //Mantis
    GO_SMALL_MANTIS_BOX             = 221822, 
    GO_MEDIUM_MANTIS_BOX            = 221820,
    GO_BIG_MANTIS_BOX               = 221804,
    //Pandaren
    GO_PANDAREN_RELIC_BOX           = 221878,
    //

    GO_SP_EX_DOOR                   = 223058, //Pre Enter Thok

    //Thok
    GO_THOK_ENT_DOOR               = 223805,
    GO_ICE_TOMB                    = 218627,
    //Jails
    GO_JINUI_JAIL                  = 222010, //right
    GO_JINUI_JAIL2                 = 222011, 

    GO_SAUROK_JAIL                 = 222046, //left
    GO_SAUROK_JAIL2                = 222047,

    GO_YAUNGOLIAN_JAIL             = 223005,
    GO_YAUNGOLIAN_JAIL2            = 223006,

    //Paragons of the Klaxxi
    GO_PRE_ENT_KLAXXI_DOOR         = 221912,
    GO_ARENA_WALL                  = 221264,
    GO_KLAXXI_EX_DOOR              = 223235,

    //Garrosh
    GO_M_HEART_OF_YSHAARJ          = 221995,
    GO_GARROSH_ENT_DOOR            = 221792,
    GO_GARROSH_FENCH               = 221441,
    GO_GARROSH_FENCH2              = 221438,
    GO_SOLDIER_RIGHT_DOOR          = 221439,
    GO_SOLDIER_LEFT_DOOR           = 221440,
    GO_SHA_VORTEX                  = 221132,
};

enum esSpells
{
    SPELL_HEWN                         = 143275,
    SPELL_ANGEL_OF_DEATH               = 141859,
    SPELL_REAVE_PL                     = 142269,
    SPELL_PLAYER_REAVE                 = 142272,
    SPELL_COMPOUND_EYE                 = 141852,
    SPELL_SNIPE                        = 143217,
    SPELL_GENETIC_ALTERATION           = 143279,
    SPELL_MAD_SCIENTIST                = 141857,
    SPELL_GENE_SPLICE                  = 143372,
    SPELL_GENE_SPLICE_PLAYER           = 143373,
    SPELL_APOTHECARIAL_KNOWLEDGE       = 142546,
    SPELL_VAST_APOTHECARIAL_KNOWLEDGE  = 141856,
    SPELL_APOTHECARY_VOLATILE_POULTICE = 142598,
    SPELL_VOLATILE_POULTICE            = 142877,
    SPELL_EXPOSED_VEINS                = 142931,
    SPELL_TENDERIZING_STRIKES_DMG      = 142929,
    SPELL_HUNGER                       = 143358,
    SPELL_TOXIN_RED                    = 142533,
    SPELL_TOXIN_BLUE                   = 142532,
    SPELL_TOXIN_YELLOW                 = 142534,
    SPELL_TOXIN_ORANGE                 = 142547,
    SPELL_TOXIN_PURPLE                 = 142548,
    SPELL_TOXIN_GREEN                  = 142549,
    SPELL_INJECTION                    = 143339,
    SPELL_CAUSTIC_AMBER_AURA_DMG       = 143735,
    SPELL_AIM_PLAYER                   = 143216,
    SPELL_MASTER_OF_AMBER              = 141854,
    SPELL_MASTER_OF_AMBER2             = 142616,
    SPELL_SUM_HUNGRY_KUNCHONG          = 146891,
    SPELL_CANNED_HEAT_BASE             = 143572,
    SPELL_TORMENT_MAIN                 = 142983,

    //Spoils of Pandaria
    SPELL_MOGU_RUNE_OF_POWER_AURA      = 145458,

    //Blackfuse
    SPELL_PROTECTIVE_FRENZY            = 145365,
    SPELL_ENERGIZED_DEFENSIVE_MATRIX   = 148990,
    SPELL_ON_CONVEYOR                  = 144287,
    SPELL_ELECTROMAGNETIC_BARRIER      = 145154,
    SPELL_ELECTROMAGNETIC_BARRIER_V    = 146663,
    SPELL_SUPERHEATED_CRAWLER_MINE     = 146342,

    //Garrosh
    SPELL_YSHAARJ_PROTECTION_AT        = 144920,
    SPELL_YSHAARJ_PROTECTION           = 144945,
    SPELL_ULTIMATE_DESPAIR             = 147341,
    SPELL_CONSUMED_FAITH               = 149033,
    SPELL_CONSUMED_COURAGE             = 149011,
    SPELL_REALM_OF_YSHAARJ             = 144954,
    SPELL_TOUCH_OF_YSHAARJ             = 145065,
    SPELL_EM_TOUCH_OF_YSHAARJ          = 145171,
    SPELL_CRUSHING_FEAR                = 147319,
    //Movie Garrosh
    SPELL_HORDE                        = 149978,
    SPELL_ALLIANCE                     = 149979,
};


enum GalakrasEvent
{
    TRANSPORT_PERIOD                = 98995,
    
    //Galakras
    ACTION_GALAKRAS_START_EVENT         = 1,
    ACTION_DEMOLITIONS_SOUTH            = 2,
    ACTION_DEMOLITIONS_NORTH            = 3,
    ACTION_DEMOLITIONS_COMPLETE         = 4,

    //Heroic summons
    ACTION_GRUNT_SOUTH                  = 5,
    ACTION_GRUNT_NORTH                  = 6,
    ACTION_GRUNT_SOUTH_FINISH           = 7,
    ACTION_GRUNT_NORTH_FINISH           = 8,

    //Galakras Guards Towers
    ACTION_TOWER_DESPAWN                = 9,
    ACTION_TOWER_GUARDS                 = 10,
    ACTION_TOWER_TURRET                 = 11,
    
    ACTION_PRE_EVENT                    = 12,
    ACTION_PRE_EVENT_FINISH             = 13,

    //Galakras Friendly Force
    ACTION_VARIAN_OR_LORTHEMAR_EVENT    = 1,
    ACTION_FRIENDLY_BOSS                = 2,
};

enum GalakrasFinishEvent
{
    //Galakras Horde Event
    EVENT_FINISH_1_H                      = 1,
    EVENT_FINISH_2_H                      = 2,
    EVENT_FINISH_3_H                      = 3,
    //Galakras Alliance Event
    EVENT_FINISH_1_A                      = 4,
    EVENT_FINISH_2_A                      = 5,
    EVENT_FINISH_3_A                      = 6,
};

static uint8 vehSlotForMeasures(uint32 entry)
{
    switch(entry)
    {
        case NPC_EMBODIED_DESPERATION_OF_SUN:
        case NPC_EMBODIED_MISERY_OF_ROOK:
            return 1;
        case NPC_EMBODIED_GLOOM_OF_ROOK:
            return 2;
    }
    return 0;
}
#endif SIEGEOFORGRIMMAR
