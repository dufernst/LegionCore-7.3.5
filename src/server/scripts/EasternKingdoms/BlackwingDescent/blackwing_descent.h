#ifndef DEF_BLACKWING_DESCENT_H
#define DEF_BLACKWING_DESCENT_H

#define BDScriptName "instance_blackwing_descent"

enum Data
{
    DATA_MAGMAW                     = 1, 
    DATA_OMNOTRON                   = 2,
    DATA_MALORIAK                   = 3,
    DATA_CHIMAERON                  = 4,
    DATA_ATRAMEDES                  = 5,
    DATA_NEFARIAN                   = 6,
    DATA_ARCANOTRON                 = 7,
    DATA_ELECTRON                   = 8,
    DATA_MAGMATRON                  = 9,
    DATA_TOXITRON                   = 10,
    DATA_HEALTH_OMNOTRON_SHARED     = 11,
    DATA_MALORIAK_ABERRATIONS       = 12,
    DATA_BILE_O_TRON_800            = 13,
    DATA_INNER_CHAMBER_DOOR         = 14,
    DATA_MAGMAW_HEAD                = 15,
    DATA_ONYXIA                     = 16,
    DATA_NEFARIAN_FLOOR             = 17,
};

enum Creatures
{
    NPC_MAGMAW                      = 41570,
    NPC_MAGMAW_HEAD                 = 42347,
    NPC_OMNOTRON                    = 42186,
    NPC_ARCANOTRON                  = 42166, 
    NPC_ELECTRON                    = 42179, 
    NPC_MAGMATRON                   = 42178, 
    NPC_TOXITRON                    = 42180, 
    NPC_MALORIAK                    = 41378,
    NPC_ATRAMEDES                   = 41442,
    NPC_CHIMAERON                   = 43296, 
    NPC_BILE_O_TRON_800             = 44418, 

    NPC_NEFARIAN                    = 41376, 
    NPC_ONYXIA                      = 41270, 

    NPC_LORD_VICTOR_NEFARIAN        = 41379,
    NPC_LORD_VICTOR_NEFARIUS_HEROIC = 49226,
    NPC_MALORIAK_EVENT              = 43404,
    NPC_ATRAMEDES_EVENT             = 43407,
};

enum GOs
{
    GO_INNER_CHAMBER_DOOR           = 205830,
    GO_NEFARIAN_FLOOR               = 207834,
};

enum NefariusActions
{
    //omnotron
    ACTION_GRIP_OF_DEATH            = 1,
    ACTION_SHADOW_INFUSION          = 2,
    ACTION_OVERCHARGE               = 3,
    ACTION_ENCASING_SHADOWS         = 4,
    ACTION_OMNOTRON_DEATH           = 5,
    ACTION_OMNOTRON_INTRO           = 6,
    //magmaw
    ACTION_SHADOW_BREATH            = 7,
    ACTION_BLAZING_INFERNO          = 8,
    ACTION_MAGMAW_INTRO             = 9,
    ACTION_MAGMAW_DEATH             = 10,
    //maloriak
    ACTION_MALORIAK_DARK_MAGIC      = 11,
    ACTION_MALORIAK_INTRO           = 12,
    ACTION_MALORIAK_DEATH           = 13,
    //chimaeron
    ACTION_CHIMAERON_INTRO          = 14,
    ACTION_CHIMAERON_DEATH          = 15,
    ACTION_CHIMAERON_FEUD           = 16,
    ACTION_CHIMAERON_LOW            = 17,
};

#endif