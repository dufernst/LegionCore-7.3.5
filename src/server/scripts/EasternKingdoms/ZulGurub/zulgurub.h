#ifndef DEF_ZULGURUB_H
#define DEF_ZULGURUB_H

#define ZGScriptName "instance_zulgurub"

uint32 const EncounterCount = 5;

enum DataTypes
{
    DATA_VENOXIS            = 0,
    DATA_MANDOKIR           = 1,
    DATA_KILNARA            = 2,
    DATA_ZANZIL             = 3,
    DATA_JINDO              = 4,

    // Cache of Madness
    DATA_HAZZARAH           = 5,
    DATA_RENATAKI           = 6,
    DATA_WUSHOOLAY          = 7,
    DATA_GRILEK             = 8,
    DATA_BOSSES             = 9,
};

enum CreatureIds
{
    NPC_VENOXIS             = 52155,
    NPC_MANDOKIR            = 52151,
    NPC_KILNARA             = 52059,
    NPC_ZANZIL              = 52053,
    NPC_JINDO               = 52148,

    // Cache of Madness
    NPC_HAZZARAH            = 52271,
    NPC_RENATAKI            = 52269,
    NPC_WUSHOOLAY           = 52286,
    NPC_GRILEK              = 52258,
};

enum GameObjectIds
{
    GO_VENOXIS_EXIT     = 208844,
    GO_MANDOKIR_EXIT1   = 208845,
    GO_MANDOKIR_EXIT2   = 208846,
    GO_MANDOKIR_EXIT3   = 208847,
    GO_MANDOKIR_EXIT4   = 208848,
    GO_MANDOKIR_EXIT5   = 208849,
    GO_ZANZIL_EXIT      = 208850,
    GO_KILNARA_EXIT     = 180497,
};

enum OtherSpells
{
    SPELL_FROSTBURN_FORMULA = 96331,
    SPELL_HYPPOTHERMIA      = 96332,
};

#endif

