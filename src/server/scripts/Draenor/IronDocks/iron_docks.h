/*
    Dungeon : Iron Docks 93-95
*/

#ifndef IRON_DOCKS_H_
#define IRON_DOCKS_H_

enum eData
{
    DATA_NOKGAR         = 0,
    DATA_G_ENFORCERS    = 1,
    DATA_OSHIR          = 2,
    DATA_SKULLOC        = 3,
    MAX_ENCOUNTER,

    DATA_G_ENFOR_DIED,
    DATA_OSHIR_CAGE,

    DATA_CAPTAIN_TEXT_1,
    DATA_CAPTAIN_TEXT_3,
    DATA_CAPTAIN_TEXT_4,
    DATA_CAPTAIN_TEXT_5,
    DATA_CAPTAIN_TEXT_6,
};

enum eCreatures
{
    //Fleshrender Nok'gar
    NPC_SHREDDING_SWIPES      = 81832,
    NPC_GROMKAR_FLAMESLINGER  = 81279,
    //Grimrail Enforcers
    NPC_MAKOGG_EMBERBLADE     = 80805,
    NPC_NEESA_NOX             = 80808,
    NPC_AHRIOK_DUGRU          = 80816,
    NPC_BOMBSQUAD             = 80875,
    //Oshir
    NPC_OSHIR                 = 79852,
    NPC_RYLAK_CAGE            = 89021,
    NPC_WOLF_CAGE             = 89022,
    NPC_RYLAK_SKYTERROR       = 89011,
    NPC_RAVENOUS_WOLF         = 89012,
    NPC_RENDING_SLASHES       = 79889,
    //Skulloc
    NPC_SKULLOC               = 83612,
    NPC_KORAMAR               = 83613,
    NPC_BLACKHAND_TURRET      = 84215,
    NPC_ZOGGOSH               = 83616,
    NPC_BACKDRAFT             = 84464,
};

enum eGameObjects
{
    GO_OSHIR_1  = 239215,
    GO_OSHIR_2  = 239216,
    GO_OSHIR_3  = 239217,
    GO_OSHIR_4  = 239218,
    GO_OSHIR_5  = 239219,
    GO_OSHIR_6  = 239220,
    GO_OSHIR_7  = 239221,
    GO_OSHIR_8  = 239222,
    GO_OSHIR_9  = 239223,
    GO_OSHIR_10 = 239224,
    GO_OSHIR_11 = 239225,
    GO_OSHIR_12 = 239226,
    GO_OSHIR_13 = 239227,
    GO_OSHIR_14 = 239228,
    GO_OSHIR_15 = 239229,
    GO_OSHIR_16 = 239230,
    GO_OSHIR_17 = 239231,
    GO_OSHIR_18 = 239232,
    GO_OSHIR_19 = 239233,
};

enum dungeonSpells
{
    SPELL_IRON_DOCKS_BANTER_1 = 168765,
    SPELL_IRON_DOCKS_BANTER_2 = 168768, //?
    SPELL_IRON_DOCKS_BANTER_3 = 168769,
    SPELL_IRON_DOCKS_BANTER_4 = 168778,
    SPELL_IRON_DOCKS_BANTER_5 = 168784,
    SPELL_IRON_DOCKS_BANTER_6 = 168785,
    SPELL_IRON_DOCKS_BANTER_7 = 168787,
};

Position const cageSpawn[19] =
{
    {924.45f, -1073.08f, 4.52119f},
    {6905.5f, -1096.7f, 4.91667f},
    {6905.34f, -1108.77f, 4.91667f},
    {6904.55f, -1133.03f, 4.91667f},
    {6962.14f, -1074.74f, 4.91667f},
    {6950.18f, -1074.69f, 4.91667f},
    {6934.44f, -1076.2f, 4.91667f},
    {6998.02f, -1151.56f, 4.91667f},
    {6996.94f, -1176.43f, 4.91667f},
    {6983.62f, -1123.81f, 4.91667f},
    {6984.08f, -1111.06f, 4.91667f},
    {6982.26f, -1098.03f, 4.91667f},
    {6976.98f, -1082.11f, 4.49775f},
    {6907.66f, -1194.63f, 4.91667f},
    {6920.18f, -1192.82f, 4.91667f},
    {6935.86f, -1194.8f, 4.91667f},
    {6964.68f, -1200.26f, 4.91667f},
    {6978.11f, -1197.16f, 4.91667f},
    {6991.88f, -1190.86f, 4.91667f},
};

#endif