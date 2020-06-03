/*
    Dungeon : Eye of Azshara 100-110
*/

#ifndef EYE_OF_AZSHARA_H_
#define EYE_OF_AZSHARA_H_

enum eData
{
    DATA_PARJESH                = 0,
    DATA_HATECOIL               = 1,
    DATA_DEEPBEARD              = 2,
    DATA_SERPENTRIX             = 3,
    DATA_WRATH_OF_AZSHARA       = 4,
    MAX_ENCOUNTER,

    DATA_WIND_ACTIVE
};

enum eCreatures
{
    //Parjesh
    NPC_WARLORD_PARJESH         = 91784,
    NPC_HATECOIL_SHELLBREAKER   = 97264,
    NPC_HATECOIL_CRESTRIDER     = 97269,

    //Hatecoil
    NPC_LADY_HATECOIL           = 91789,
    NPC_HATECOIL_ARCANIST       = 97171,
    NPC_SAND_DUNE               = 97853,
    NPC_MONSOON                 = 99852,

    //Deepbeard
    NPC_KING_DEEPBEARD          = 91797,
    NPC_QUAKE                   = 97916,

    //Serpentrix
    NPC_SERPENTRIX              = 91808,
    NPC_BLAZING_HYDRA_SPAWN     = 97259,
    NPC_ARCANE_HYDRA_SPAWN      = 97260,

    //Wrath of Azshara
    NPC_WRATH_OF_AZSHARA        = 96028,
    NPC_WEATHERMAN              = 97063,
    NPC_LIGHTNING_STALKER       = 97713,
    NPC_MYSTIC_SSAVEH           = 98173, 
    NPC_RITUALIST_LESHA         = 100248,
    NPC_CHANNELER_VARISZ        = 100249,
    NPC_BINDER_ASHIOI           = 100250,
    NPC_ARCANE_BOMB             = 97691,
    NPC_TIDAL_WAVE              = 97739,

    NPC_TIDESTONE_OF_GOLGANNETH = 106780,
};

enum eGameObjects
{
    GO_SAND_DUNE                = 244690,
    GO_DEEPBEARD_DOOR           = 246983,
    GO_AZSHARA_BUBBLE           = 240788,
};

enum miscSpells
{
    SPELL_SKYBOX_RAIN           = 191815,
    SPELL_SKYBOX_LIGHTNING      = 191816,
    SPELL_SKYBOX_WIND           = 212614,
    SPELL_SKYBOX_HURRICANE      = 212615,
};

#endif