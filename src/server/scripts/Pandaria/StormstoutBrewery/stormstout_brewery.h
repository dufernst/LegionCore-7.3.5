/*=======================
========================*/

#ifndef STORMSTOUT_BREWERY_H_
#define STORMSTOUT_BREWERY_H_
#define SBScriptName "instance_stormstout_brewery"

enum DataTypes
{
    DATA_OOK_OOK   = 0,
    DATA_HOPTALLUS = 1,
    DATA_YAN_ZHU   = 2,

    DATA_HOPLING,
    DATA_GOLDEN_HOPLING
};

enum CreaturesIds
{
    NPC_OOK_OOK     = 56637,
    NPC_HOPTALLUS   = 56717,
    NPC_YAN_ZHU     = 59479
};

enum GameObjectIds
{
    GO_EXIT_OOK_OOK  = 211132,
    GO_DOOR          = 211134,
    GO_DOOR2         = 211133,
    GO_DOOR3         = 211135,
    GO_DOOR4         = 211137,
    GO_LAST_DOOR     = 211136,
    GO_CARROT_DOOR   = 211126
};

enum eMisc
{
    NPC_BARREL                  = 56731,

    //Pre event Hopper Summons (114363,114553)
    NPC_HOPPER_HAMMER           = 59426,
    NPC_BOPPER_EXPLOSIVE        = 56718,
    
    //Hoptallus Summons 
    NPC_TRIGGER_SUMMONER        = 55091,
    NPC_BOPPER_HAMMER           = 59551,
    NPC_HOPPER_EXPLOSIVE        = 59464,
    
    SPELL_PROC_EXPLOSION        = 106787,
    SPELL_GOLDEN_VERMING_ACHIEV = 116270,
    SPELL_SMASH_OVERRIDE        = 111662,
    SPELL_SMASH_DMG             = 111666,
    SPELL_HOPPER_SUM_EXPLOSIVE  = 114363,
    SPELL_HOPPER_SUM_HAMMER     = 114553,
    SPELL_SMASH_ACHIEV          = 116286,
    SPELL_HOPLING_AURA_3        = 114357,
    SPELL_HOPLING_SUMM_3        = 114356
};

#endif
