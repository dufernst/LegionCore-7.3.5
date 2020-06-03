#ifndef DEF_BASTION_OF_TWILIGHT_H
#define DEF_BASTION_OF_TWILIGHT_H

#define BTScriptName "instance_bastion_of_twilight"

enum Data
{
    DATA_HALFUS                     = 0,
    DATA_VALIONA_THERALION          = 1,
    DATA_COUNCIL                    = 2,
    DATA_CHOGALL                    = 3,
    DATA_SINESTRA                   = 4,
    DATA_TEAM_IN_INSTANCE           = 5,
    DATA_VALIONA                    = 6,
    DATA_THERALION                  = 7,
    DATA_FELUDIUS                   = 8,
    DATA_IGNACIOUS                  = 9,
    DATA_ARION                      = 10,
    DATA_TERRASTRA                  = 11,
    DATA_MONSTROSITY                = 12,
    DATA_WHELP_CAGE                 = 13,
    DATA_HEALTH_VALIONA_THERALION   = 14,
    DATA_DLG_SINESTRA               = 15,
    DATA_DLG_HALFUS                 = 16,
    DATA_DLG_VALIONA_THERALION      = 17,
    DATA_DLG_CHOGALL                = 18,
    DATA_DLG_COUNCIL_1              = 19,
    DATA_DLG_COUNCIL_2              = 20,
    DATA_DLG_COUNCIL_3              = 21,
    DATA_DLG_ENTRANCE               = 22,
};

enum CreatureIds
{
    NPC_HALFUS_WYRMBREAKER  = 44600,
    NPC_VALIONA             = 45992,
    NPC_THERALION           = 45993,
    NPC_FELUDIUS            = 43687,
    NPC_IGNACIOUS           = 43686,
    NPC_ARION               = 43688,
    NPC_TERRASTRA           = 43689,
    NPC_MONSTROSITY         = 43735,
    NPC_CHOGALL             = 43324,
    NPC_SINESTRA            = 45213,
    NPC_CHOGALL_DLG         = 46965,    
};

enum GameObjects
{
    GO_WHELP_CAGE           = 205087,
    GO_WHELP_CAGE2          = 205088,
    DOOR_HALFUS_ENTRANCE    = 205222,
    DOOR_HALFUS_EXIT        = 205223,
    DOOR_THER_ENTRANCE      = 205224,
    DOOR_THER_EXIT          = 205225,
    DOOR_COUNCIL_ENTRANCE   = 205226,
    DOOR_COUNCIL_EXIT       = 205227,
    DOOR_CHOGALL_ENTRANCE   = 205228,
    GO_CHOGALL_FLOOR        = 205898,
    GO_SINESTRA_DOOR        = 207679,
};

enum ActionsDlg
{
    ACTION_AT_ENTRANCE                      = 1, // 6341
    ACTION_AT_HALFUS_START                  = 2, // 6437
    ACTION_AT_HALFUS_END                    = 3, 
    ACTION_AT_VALIONA_THERALION_START       = 4, // 6442
    ACTION_AT_VALIONA_THERALION_END         = 5, 
    ACTION_AT_COUNCIL_1                     = 6, // 6625
    ACTION_AT_COUNCIL_2                     = 7, // 6626
    ACTION_AT_COUNCIL_3                     = 8, // 6627 
    ACTION_AT_CHOGALL                       = 9, // 6444
};

const Position enterPos = {-548.15f, -532.34f, 890.60f, 0.0f};

#endif