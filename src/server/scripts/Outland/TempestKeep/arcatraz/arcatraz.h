#ifndef ARCATRAZ_H
#define ARCATRAZ_H

#include "CreatureAIImpl.h"

#define ArcatrazScriptName "instance_arcatraz"

uint32 const EncounterCount = 4;

enum AZDataTypes
{
    // Encounter States/Boss GUIDs
    DATA_ZEREKETH = 0,
    DATA_DALLIAH = 1,
    DATA_SOCCOTHRATES = 2,
    DATA_HARBINGER_SKYRISS = 3,

    // Additional Data
    DATA_CONVERSATION = 4,
    DATA_WARDEN_1 = 5, // used by SmartAI
    DATA_WARDEN_2 = 6, // used by SmartAI
    DATA_WARDEN_3 = 7, // used by SmartAI
    DATA_WARDEN_4 = 8, // used by SmartAI
    DATA_WARDEN_5 = 9, // used by SmartAI
    DATA_MELLICHAR = 10,
    DATA_WARDENS_SHIELD = 11
};

enum AZCreatureIds
{
    NPC_DALLIAH = 20885,
    NPC_SOCCOTHRATES = 20886,
    NPC_MELLICHAR = 20904, // skyriss will kill this unit
    NPC_ALPHA_POD_TARGET = 21436
};

enum AZGameObjectIds
{
    GO_CONTAINMENT_CORE_SECURITY_FIELD_ALPHA = 184318, // door opened when Wrath-Scryer Soccothrates dies
    GO_CONTAINMENT_CORE_SECURITY_FIELD_BETA = 184319, // door opened when Dalliah the Doomsayer dies
    GO_STASIS_POD_ALPHA = 183961, // pod first boss wave
    GO_STASIS_POD_BETA = 183963, // pod second boss wave
    GO_STASIS_POD_DELTA = 183964, // pod third boss wave
    GO_STASIS_POD_GAMMA = 183962, // pod fourth boss wave
    GO_STASIS_POD_OMEGA = 183965, // pod fifth boss wave
    GO_WARDENS_SHIELD = 184802  // shield 'protecting' mellichar
};

template<typename AI>
inline AI* GetArcatrazAI(Creature* creature)
{
    return GetInstanceAI<AI>(creature, ArcatrazScriptName);
}

#endif // ARCATRAZ_H