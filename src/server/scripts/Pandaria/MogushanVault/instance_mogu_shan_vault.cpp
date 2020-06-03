
//Mogushan Vault

#include "mogu_shan_vault.h"

const DoorData doorData[] =
{
    {GOB_STONE_GUARD_DOOR_EXIT,    DATA_STONE_GUARD,   DOOR_TYPE_PASSAGE, 0},
    {GOB_FENG_DOOR_EXIT,           DATA_FENG,          DOOR_TYPE_PASSAGE, 0},
    {GOB_GARAJAL_EXIT,             DATA_GARAJAL,       DOOR_TYPE_PASSAGE, 0},
    {GOB_SPIRIT_KINGS_EXIT,        DATA_SPIRIT_KINGS,  DOOR_TYPE_PASSAGE, 0},
    {GOB_ELEGON_DOOR_ENTRANCE,     DATA_SPIRIT_KINGS,  DOOR_TYPE_PASSAGE, 0},
    {GOB_WILL_OF_EMPEROR_ENTRANCE, DATA_ELEGON,        DOOR_TYPE_PASSAGE, 0},
    {0,                            0,                  DOOR_TYPE_PASSAGE, 0}
};

class instance_mogu_shan_vault : public InstanceMapScript
{
public:
    instance_mogu_shan_vault() : InstanceMapScript("instance_mogu_shan_vault", 1008) { }


    InstanceScript* GetInstanceScript(InstanceMap* map) const override
    {
        return new instance_mogu_shan_vault_InstanceMapScript(map);
    }

    struct instance_mogu_shan_vault_InstanceMapScript : public InstanceScript
    {
        instance_mogu_shan_vault_InstanceMapScript(Map* map) : InstanceScript(map) {}

        //GameObject
        ObjectGuid stoneexitdoorGuid;
        ObjectGuid stoneentrdoorGuid;
        ObjectGuid fengexitdoorGuid;
        ObjectGuid garajalexitdoorGuid;
        ObjectGuid spiritexitdoorGuid;
        ObjectGuid elegonentdoorGuid;
        ObjectGuid elegonceldoorGuid;
        ObjectGuid elegonplatformGuid;
        ObjectGuid imperatorentdoorGuid;

        //Creature
        ObjectGuid stoneGuardControlerGuid;
        ObjectGuid fengGuid;
        ObjectGuid inversionGobGuid;
        ObjectGuid cancelGobGuid;
        ObjectGuid spiritKingsControlerGuid;
        ObjectGuid qiangGuid;
        ObjectGuid subetaiGuid;
        ObjectGuid zianGuid;
        ObjectGuid mengGuid;
        ObjectGuid elegonGUID;
        ObjectGuid janxiGuid;
        ObjectGuid qinxiGuid;
        ObjectGuid woicontrollerGuid;

        //Arrays
        GuidVector stoneGuardGUIDs;
        GuidVector fengdoorGUIDs;
        GuidVector garajaldoorGUIDs;
        GuidVector fengStatuesGUIDs;
        GuidVector kingsdoorGUIDs;

        void Initialize() override
        {
            SetBossNumber(DATA_MAX_BOSS_DATA);
            LoadDoorData(doorData);

            //GameObject
            stoneexitdoorGuid.Clear();
            stoneentrdoorGuid.Clear();
            fengexitdoorGuid.Clear();
            garajalexitdoorGuid.Clear();
            spiritexitdoorGuid.Clear();
            elegonentdoorGuid.Clear();
            elegonceldoorGuid.Clear();
            elegonplatformGuid.Clear();
            imperatorentdoorGuid.Clear();

            //Creature
            stoneGuardControlerGuid.Clear();
            fengGuid.Clear();
            inversionGobGuid.Clear();
            cancelGobGuid.Clear();
            spiritKingsControlerGuid.Clear();
            qiangGuid.Clear();
            subetaiGuid.Clear();
            zianGuid.Clear();
            mengGuid.Clear();
            elegonGUID.Clear();
            woicontrollerGuid.Clear();

            //Arrays
            stoneGuardGUIDs.clear();
            fengStatuesGUIDs.clear();
            garajaldoorGUIDs.clear();
            fengdoorGUIDs.clear();
            kingsdoorGUIDs.clear();
        }
        
        void OnCreatureCreate(Creature* creature) override
        {
            switch (creature->GetEntry())
            {
                case NPC_STONE_GUARD_CONTROLER:
                    stoneGuardControlerGuid = creature->GetGUID();
                    break;
                case NPC_JASPER:
                case NPC_JADE:
                case NPC_AMETHYST:
                case NPC_COBALT:
                    stoneGuardGUIDs.push_back(creature->GetGUID());
                    break;
                case NPC_FENG:
                    fengGuid = creature->GetGUID();
                    break;
                case NPC_SPIRIT_GUID_CONTROLER:
                    spiritKingsControlerGuid = creature->GetGUID();
                    break;
                case NPC_QIANG:
                    qiangGuid = creature->GetGUID();
                    break;
                case NPC_ZIAN:
                    zianGuid = creature->GetGUID();
                    break;
                case NPC_MENG:
                    mengGuid = creature->GetGUID();
                    break;
                case NPC_SUBETAI:
                    subetaiGuid = creature->GetGUID();
                    break;
                case NPC_ELEGON:
                    elegonGUID = creature->GetGUID();
                    break;
                case NPC_QIN_XI:
                    qinxiGuid = creature->GetGUID();
                    break;
                case NPC_JAN_XI:
                    janxiGuid = creature->GetGUID();
                    break;
                case NPC_WOI_CONTROLLER:
                    woicontrollerGuid = creature->GetGUID();
                    break;
                default:
                    break;
            }
        }

        void OnGameObjectCreate(GameObject* go) override
        {
            switch (go->GetEntry())
            {
                case GOB_STONE_GUARD_DOOR_ENTRANCE:
                    stoneentrdoorGuid = go->GetGUID();
                    break;
                case GOB_STONE_GUARD_DOOR_EXIT:
                    AddDoor(go, true);
                    stoneexitdoorGuid = go->GetGUID();
                    break;
                case GOB_FENG_DOOR_FENCE:
                    fengdoorGUIDs.push_back(go->GetGUID());
                    break;
                case GOB_SPEAR_STATUE:
                case GOB_FIST_STATUE:
                case GOB_SHIELD_STATUE:
                case GOB_STAFF_STATUE:
                    fengStatuesGUIDs.push_back(go->GetGUID());
                    break;
                case GOB_INVERSION:
                    inversionGobGuid = go->GetGUID();
                    break;
                case GOB_CANCEL:
                    cancelGobGuid = go->GetGUID();
                    break;
                case GOB_FENG_DOOR_EXIT:
                    AddDoor(go, true);
                    fengexitdoorGuid = go->GetGUID();
                    break;
                case GOB_GARAJAL_FENCE:
                    garajaldoorGUIDs.push_back(go->GetGUID());
                    break;
                case GOB_GARAJAL_EXIT:
                    AddDoor(go, true);
                    garajalexitdoorGuid = go->GetGUID();
                    break;
                case GOB_SPIRIT_KINGS_WIND_WALL:
                    kingsdoorGUIDs.push_back(go->GetGUID());
                    break;
                case GOB_SPIRIT_KINGS_EXIT:
                    AddDoor(go, true);
                    spiritexitdoorGuid = go->GetGUID();
                    break;
                case GOB_ELEGON_DOOR_ENTRANCE:
                    AddDoor(go, true);
                    elegonentdoorGuid = go->GetGUID();
                    break;
                case GOB_ELEGON_CELESTIAL_DOOR:
                    elegonceldoorGuid = go->GetGUID();
                    break;
                case GOB_ENERGY_PLATFORM:
                    elegonplatformGuid = go->GetGUID();
                    break;
                case GOB_WILL_OF_EMPEROR_ENTRANCE:
                    AddDoor(go, true);
                    imperatorentdoorGuid = go->GetGUID();
                    break;
            }
        }
        
        bool SetBossState(uint32 id, EncounterState state) override
        {
            if (!InstanceScript::SetBossState(id, state))
                return false;

            switch (id)
            {
                case DATA_STONE_GUARD:
                {
                    switch (state)
                    {
                        case FAIL:
                        {
                            for (GuidVector::const_iterator guid = stoneGuardGUIDs.begin(); guid != stoneGuardGUIDs.end(); ++guid)
                                if (Creature* stoneGuard = instance->GetCreature(*guid))
                                    stoneGuard->AI()->DoAction(ACTION_FAIL);

                            HandleGameObject(stoneentrdoorGuid, true);
                        }
                        break;
                        case IN_PROGRESS:
                            HandleGameObject(stoneentrdoorGuid, false);
                            break;
                        case DONE:
                            HandleGameObject(stoneexitdoorGuid, true);
                            HandleGameObject(stoneentrdoorGuid, true);
                            break;
                    }
                    break;
                }
                case DATA_FENG:
                    {
                        switch (state)
                        {
                        case NOT_STARTED:
                            for (GuidVector::const_iterator guid = fengdoorGUIDs.begin(); guid != fengdoorGUIDs.end(); guid++)
                                HandleGameObject(*guid, true);
                            break;
                        case DONE:
                            for (GuidVector::const_iterator guid = fengdoorGUIDs.begin(); guid != fengdoorGUIDs.end(); guid++)
                                HandleGameObject(*guid, true);
                            HandleGameObject(fengexitdoorGuid, true);
                            break;
                        case IN_PROGRESS:
                            for (GuidVector::const_iterator guid = fengdoorGUIDs.begin(); guid != fengdoorGUIDs.end(); guid++)
                                HandleGameObject(*guid, false);
                            break;
                        }
                        break;
                    }
                case DATA_GARAJAL:
                    {
                        switch (state)
                        {
                        case NOT_STARTED:
                            for (GuidVector::const_iterator guid = garajaldoorGUIDs.begin(); guid != garajaldoorGUIDs.end(); guid++)
                                HandleGameObject(*guid, true);
                            break;
                        case IN_PROGRESS:
                            for (GuidVector::const_iterator guid = garajaldoorGUIDs.begin(); guid != garajaldoorGUIDs.end(); guid++)
                                HandleGameObject(*guid, false);
                            break;
                        case DONE:
                            for (GuidVector::const_iterator guid = garajaldoorGUIDs.begin(); guid != garajaldoorGUIDs.end(); guid++)
                                HandleGameObject(*guid, true);
                            HandleGameObject(garajalexitdoorGuid, true);
                            break;
                        }
                        break;
                    }
                case DATA_SPIRIT_KINGS:
                    {
                        switch (state)
                        {
                        case NOT_STARTED:
                            for (GuidVector::const_iterator guid = kingsdoorGUIDs.begin(); guid != kingsdoorGUIDs.end(); guid++)
                                HandleGameObject(*guid, true);
                            break;
                        case IN_PROGRESS:
                            for (GuidVector::const_iterator guid = kingsdoorGUIDs.begin(); guid != kingsdoorGUIDs.end(); guid++)
                                HandleGameObject(*guid, false);
                            break;
                        case DONE:
                            for (GuidVector::const_iterator guid = kingsdoorGUIDs.begin(); guid != kingsdoorGUIDs.end(); guid++)
                                HandleGameObject(*guid, true);
                            HandleGameObject(spiritexitdoorGuid, true);
                            HandleGameObject(elegonentdoorGuid, true);
                            break;
                        }
                    break;
                    }
                case DATA_ELEGON:
                    {
                        switch (state)
                        {
                        case NOT_STARTED:
                            HandleGameObject(elegonceldoorGuid, true);
                            break;
                        case IN_PROGRESS:
                            HandleGameObject(elegonceldoorGuid, false);
                            break;
                        case DONE:
                            HandleGameObject(elegonceldoorGuid, true);
                            HandleGameObject(imperatorentdoorGuid, true);
                            break;
                        }
                        break;
                    }
                case DATA_WILL_OF_EMPEROR:
                    {
                        switch (state)
                        {
                        case NOT_STARTED:
                            HandleGameObject(imperatorentdoorGuid, true);
                            break;
                        case IN_PROGRESS:
                            HandleGameObject(imperatorentdoorGuid, false);
                            break;
                        case DONE:
                            HandleGameObject(imperatorentdoorGuid, true);
                            break;
                        }
                        break;
                    }
            }
            return true;
        }

        void SetData(uint32 type, uint32 data) override {}

        uint32 GetData(uint32 type) const override
        {
            return 0;
        }

        ObjectGuid GetGuidData(uint32 type) const override
        {
            switch (type)
            {
                case NPC_STONE_GUARD_CONTROLER:
                    return stoneGuardControlerGuid;
                case NPC_JASPER:
                case NPC_JADE:
                case NPC_AMETHYST:
                case NPC_COBALT:
                {
                    for (GuidVector::const_iterator guid = stoneGuardGUIDs.begin(); guid != stoneGuardGUIDs.end(); ++guid)
                        if (Creature* stoneGuard = instance->GetCreature(*guid))
                            if (stoneGuard->GetEntry() == type)
                                return *guid;
                    break;
                }
                case NPC_FENG:
                    return fengGuid;
                case GOB_SPEAR_STATUE:
                case GOB_FIST_STATUE:
                case GOB_SHIELD_STATUE:
                case GOB_STAFF_STATUE:
                {
                    for (GuidVector::const_iterator guid = fengStatuesGUIDs.begin(); guid != fengStatuesGUIDs.end(); ++guid)
                        if (GameObject* fengStatue = instance->GetGameObject(*guid))
                            if (fengStatue->GetEntry() == type)
                                return *guid;
                    break;
                }
                case GOB_INVERSION: 
                    return inversionGobGuid;
                case GOB_CANCEL: 
                    return cancelGobGuid;
                case NPC_SPIRIT_GUID_CONTROLER:
                    return spiritKingsControlerGuid;
                case NPC_QIANG:
                    return qiangGuid;
                case NPC_ZIAN:
                    return zianGuid;
                case NPC_MENG:
                    return mengGuid;
                case NPC_SUBETAI:
                    return subetaiGuid;
                case NPC_ELEGON:
                    return elegonGUID;
                case GOB_ENERGY_PLATFORM:
                    return elegonplatformGuid;
                case NPC_QIN_XI:
                    return qinxiGuid;
                case NPC_JAN_XI:
                    return janxiGuid;
                case NPC_WOI_CONTROLLER:
                    return woicontrollerGuid;
            }
            return ObjectGuid::Empty;
        }

        bool IsWipe() const override
        {
            Map::PlayerList const& PlayerList = instance->GetPlayers();

            if (PlayerList.isEmpty())
                return true;

            for (Map::PlayerList::const_iterator Itr = PlayerList.begin(); Itr != PlayerList.end(); ++Itr)
            {
                Player* player = Itr->getSource();

                if (!player)
                    continue;

                if (player->isAlive() && !player->isGameMaster() && !player->HasAura(115877)) // Aura 115877 = Totaly Petrified
                    return false;
            }

            return true;
        }

        std::string GetSaveData() override
        {
            std::ostringstream saveStream;
            saveStream << GetBossSaveData() << " ";
            return saveStream.str();
        }

        void Load(const char* data) override
        {
            std::istringstream loadStream(LoadBossState(data));
            uint32 buff;
            for (uint32 i=0; i < DATA_MAX_BOSS_DATA; ++i)
                loadStream >> buff;
        }
    };
};

void AddSC_instance_mogu_shan_vault()
{
    new instance_mogu_shan_vault();
}
