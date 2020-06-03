#ifndef DEF_HOUROFTWILIGHT_H
#define DEF_HOUROFTWILIGHT_H

const Position drakePos = {4292.110840f, 577.053528f, -6.63f, 3.54f};
const Position teleportPos = {3938.491211f, 274.222351f, 13.97f, 3.26f};

enum Datas
{
    DATA_ARCURION   = 0,
    DATA_ASIRA      = 1,
    DATA_BENEDICTUS = 2,
};

enum CreatureIds
{
    NPC_THRALL_1        = 54548, // on start
    NPC_THRALL_2        = 55779, // before Arcurion
    NPC_THRALL_3        = 54972, // before Asira's mobs
    NPC_THRALL_4        = 54634, // before last boss' mobs
    NPC_THRALL_5        = 54971, // fighting with last boss
    NPC_LIFE_WARDEN_1   = 55415, // for thrall
    NPC_LIFE_WARDEN_2   = 55549, // for players

    NPC_ARCURION        = 54590,
    NPC_ASIRA           = 54968,
    NPC_BENEDICTUS      = 54938,
};

enum GameObjectIds
{
    GO_PORTAL_TO_ORGRIMMAR  = 209081,
    GO_PORTAL_TO_STORMWIND  = 209080,

    GO_GATE                 = 190236,

    GO_ICEWALL_2            = 210048, // after Arcurion
    GO_ICEWALL_1            = 210049, // before Arcurion
};

enum OtherSpells
{
    SPELL_TELEPORT_ARCURION_DEATH   = 108928,
};

template<class AI>
CreatureAI* GetInstanceAI(Creature* creature)
{
    if (InstanceMap* instance = creature->GetMap()->ToInstanceMap())
        if (instance->GetInstanceScript())
            if (instance->GetScriptId() == sObjectMgr->GetScriptId("instance_hour_of_twilight"))
                return new AI(creature);
    return NULL;
}

#endif
