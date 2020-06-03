#ifndef DEF_BARADINHOLD_H
#define DEF_BARADINHOLD_H

#define MAX_ENCOUNTER 3

enum Creatures
{
    NPC_ARGALOTH            = 47120,
    NPC_OCCUTHAR            = 52363,
    NPC_ALIZABAL            = 55869,
};

enum Gameobjects
{
    GO_TOLBARAD_DOOR_2  = 207619,
    GO_CELL_DOOR        = 208953,
    GO_TOLBARAD_DOOR_1  = 209849,
};

enum Data
{
    DATA_ARGALOTH   = 0,
    DATA_OCCUTHAR   = 1,
    DATA_ALIZABAL   = 2,   
};

template<class AI>
CreatureAI* GetInstanceAI(Creature* creature)
{
    if (InstanceMap* instance = creature->GetMap()->ToInstanceMap())
        if (instance->GetInstanceScript())
            if (instance->GetScriptId() == sObjectMgr->GetScriptId("instance_baradin_hold"))
                return new AI(creature);
    return NULL;
}

#endif
