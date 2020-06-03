/*
    Dungeon : The Everbloom 100
*/

#ifndef THE_EVERBLOOM_H_
#define THE_EVERBLOOM_H_

enum eData
{
    DATA_WITHERBARK     = 0,
    DATA_PROTECTORS     = 1,
    DATA_XERITAC        = 2,
    DATA_ARCHMAGE_SOL   = 3,
    DATA_YALNU          = 4,
    MAX_ENCOUNTER,
};

enum eCreatures
{
    //WitherBark
    NPC_EVERBLOOM_NATURALIST    = 81819,
    NPC_EVERBLOOM_MENDER        = 81820,
    NPC_AQUEOUS_GLOBULE_TRIGGER = 81821,
    NPC_AQUEOUS_GLOBULE         = 81638,
    NPC_UNCHECKED_GROWTH        = 81564,
    //Defenders
    NPC_LIFE_WARDEN_GOLA        = 83892,
    NPC_EARTHSHAPER_TELU        = 83893,
    NPC_DULHU                   = 83894,
    //Archmage Sol
    NPC_PUTRID_PYROMANCER       = 84957,
    NPC_FROZEN_RAIN             = 82846,
    //Yalnu
    NPC_YALNU                   = 83846,
    NPC_LADY_BAIHU              = 84358,
    NPC_KIRIN_TOR_MAGE          = 84329,
    NPC_COLOSSAL_BLOW           = 84964,
    NPC_ENTANGLEMENT            = 84499,
    NPC_ENTANGLEMENT_PLR        = 85194,
    NPC_FONT_LIFE_STALKER       = 85107,
    NPC_VICIOUS_MANDRAGORA      = 84399,
    NPC_GNARLED_ANCIENT         = 84400,
    NPC_SWIFT_SPROUTLING        = 84401,
    NPC_FERAL_LASHER            = 86684,
};

enum eGameObjects
{
    GO_WITHERBARK_DOOR_1    = 231966,
    GO_WITHERBARK_DOOR_2    = 231967,
    GO_YALNU_VISUAL_DOOR    = 235363,
    GO_YALNU_DOOR           = 236013,
};

#endif