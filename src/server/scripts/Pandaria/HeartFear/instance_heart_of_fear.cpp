
//Heart of Fear

#include "VMapFactory.h"
#include "heart_of_fear.h"

const DoorData doorData[] =
{
    {GO_VIZIER_EX_DOOR,   DATA_VIZIER_ZORLOK, DOOR_TYPE_PASSAGE, 0},
    {GO_TAYAK_EX_DOOR,    DATA_LORD_TAYAK,    DOOR_TYPE_PASSAGE, 0},
    {GO_GARALON_ENT_DOOR, DATA_LORD_TAYAK,    DOOR_TYPE_PASSAGE, 0},
    {GO_GARALON_EX_DOOR,  DATA_GARALON,       DOOR_TYPE_PASSAGE, 0},
    {GO_MELJARAK_EX_DOOR, DATA_MELJARAK,      DOOR_TYPE_PASSAGE, 0},
    {GO_UNSOK_EN_DOOR,    DATA_MELJARAK,      DOOR_TYPE_PASSAGE, 0},
    {GO_UNSOK_EX_DOOR,    DATA_UNSOK,         DOOR_TYPE_PASSAGE, 0},
    {0,                   0,                  DOOR_TYPE_PASSAGE, 0}
};

class instance_heart_of_fear : public InstanceMapScript
{
public:
    instance_heart_of_fear() : InstanceMapScript("instance_heart_of_fear", 1009) { }


    InstanceScript* GetInstanceScript(InstanceMap* map) const
    {
        return new instance_heart_of_fear_InstanceMapScript(map);
    }

    struct instance_heart_of_fear_InstanceMapScript : public InstanceScript
    {
        instance_heart_of_fear_InstanceMapScript(Map* map) : InstanceScript(map) {}

        //GameObjects
        ObjectGuid vizierentdoorGuid;
        ObjectGuid vizierexdoorGuid;
        ObjectGuid tayakexdoorGuid;
        ObjectGuid garalonentdoorGuid;
        ObjectGuid meljarakexdoorGuid;
        ObjectGuid unsokendoorGuid;
        ObjectGuid unsokexdoorGuid;
        ObjectGuid empresscocoonGuid;

        std::vector<ObjectGuid> vizierarenadoorGuids;
        std::vector<ObjectGuid> garaloncdoorGuids;
        std::vector<ObjectGuid> garalonexdoorGuids;
        std::vector<ObjectGuid> meljarakWeaponRackGUID;

        //Creature
        ObjectGuid zorlokGuid;
        ObjectGuid gascontrollerGuid;
        ObjectGuid tayakGuid;
        ObjectGuid garalonGuid;
        ObjectGuid meljarakGuid;
        ObjectGuid unsokGuid;
        ObjectGuid ambermonsterGuid;
        ObjectGuid shekzeerGuid;

        ObjectGuid srathik[3];
        ObjectGuid zarthik[3];
        ObjectGuid korthik[3];
        std::vector<ObjectGuid> meljaraksoldiersGuids;
        
        //other
        uint8 stormUnleashed;
        
        void Initialize()
        {
            SetBossNumber(7);
            LoadDoorData(doorData);

            //GameObject
            vizierentdoorGuid       = ObjectGuid::Empty;
            vizierexdoorGuid        = ObjectGuid::Empty;
            tayakexdoorGuid         = ObjectGuid::Empty;
            garalonentdoorGuid      = ObjectGuid::Empty;
            meljarakexdoorGuid      = ObjectGuid::Empty;
            unsokendoorGuid         = ObjectGuid::Empty;
            unsokexdoorGuid         = ObjectGuid::Empty;
            empresscocoonGuid       = ObjectGuid::Empty;

            vizierarenadoorGuids.clear();
            garaloncdoorGuids.clear();
            garalonexdoorGuids.clear();
            meljarakWeaponRackGUID.clear();

            //Creature
            zorlokGuid              = ObjectGuid::Empty;
            gascontrollerGuid       = ObjectGuid::Empty;
            tayakGuid               = ObjectGuid::Empty;
            garalonGuid             = ObjectGuid::Empty;
            meljarakGuid            = ObjectGuid::Empty;
            unsokGuid               = ObjectGuid::Empty;
            ambermonsterGuid        = ObjectGuid::Empty;
            shekzeerGuid            = ObjectGuid::Empty;

            for (uint8 n = 0; n < 3; n++)
            {
                srathik[n] = ObjectGuid::Empty;
                zarthik[n] = ObjectGuid::Empty;
                korthik[n] = ObjectGuid::Empty;
            }
            //Other
            stormUnleashed = 0;
        }

        void OnPlayerLeave(Player* player)
        {
            DoRemoveAurasDueToSpellOnPlayers(126159);
        }

        void OnCreatureCreate(Creature* creature)
        {
            switch (creature->GetEntry())
            { 
                case NPC_VIZIER_ZORLOK:
                    zorlokGuid = creature->GetGUID();
                    break;
                case NPC_GAS_CONTROLLER:
                    gascontrollerGuid = creature->GetGUID();
                    break;
                case NPC_LORD_TAYAK:
                    tayakGuid = creature->GetGUID();
                    break;
                case NPC_GARALON:
                    garalonGuid = creature->GetGUID();
                    break;
                case NPC_MELJARAK:
                    meljarakGuid = creature->GetGUID();
                    break;
                case NPC_SRATHIK:
                    for (uint8 n = 0; n < 3; n++)
                    {
                        if (srathik[n].IsEmpty())
                        {
                            srathik[n] = creature->GetGUID();
                            meljaraksoldiersGuids.push_back(srathik[n]);
                            break;
                        }
                    }
                    break;
                case NPC_ZARTHIK:
                    for (uint8 n = 0; n < 3; n++)
                    {
                        if (zarthik[n].IsEmpty())
                        {
                            zarthik[n] = creature->GetGUID();
                            meljaraksoldiersGuids.push_back(zarthik[n]);
                            break;
                        }
                    }
                    break;
                case NPC_KORTHIK:
                    for (uint8 n = 0; n < 3; n++)
                    {
                        if (korthik[n].IsEmpty())
                        {
                            korthik[n] = creature->GetGUID();
                            meljaraksoldiersGuids.push_back(korthik[n]);
                            break;
                        }
                    }
                    break;
                case NPC_UNSOK:
                    unsokGuid = creature->GetGUID();
                    break;
                case NPC_AMBER_MONSTER:
                    ambermonsterGuid = creature->GetGUID();
                    break;
                case NPC_SHEKZEER:
                    shekzeerGuid = creature->GetGUID();
                    break;
            }
        }

        void OnGameObjectCreate(GameObject* go)
        {
            switch (go->GetEntry())
            {
                case GO_VIZIER_ENT_DOOR:
                    vizierentdoorGuid = go->GetGUID();
                    break;
                case GO_VIZIER_ARENA_DOOR:
                    vizierarenadoorGuids.push_back(go->GetGUID());
                    break;
                case GO_VIZIER_EX_DOOR:
                    AddDoor(go, true);
                    vizierexdoorGuid = go->GetGUID();
                    break;
                case GO_TAYAK_EX_DOOR:
                    AddDoor(go, true);
                    tayakexdoorGuid = go->GetGUID();
                    break;
                case GO_GARALON_ENT_DOOR:
                    AddDoor(go, true);
                    garalonentdoorGuid = go->GetGUID();
                    break;
                case GO_GARALON_COMBAT_DOOR:
                    garaloncdoorGuids.push_back(go->GetGUID());
                    break;
                case GO_GARALON_EX_DOOR:
                    AddDoor(go, true);
                    garalonexdoorGuids.push_back(go->GetGUID());
                    break;
                case GO_MELJARAK_EX_DOOR:
                    AddDoor(go, true);
                    meljarakexdoorGuid = go->GetGUID();
                    break;
                case GO_MELJARAK_WEAPON_RACK:
                    meljarakWeaponRackGUID.push_back(go->GetGUID());
                    break;
                case GO_UNSOK_EN_DOOR:
                    AddDoor(go, true);
                    unsokendoorGuid = go->GetGUID();
                    break;
                case GO_UNSOK_EX_DOOR:
                    AddDoor(go, true);
                    unsokexdoorGuid = go->GetGUID();
                    break;
                case GO_EMPRESS_COCOON:
                    empresscocoonGuid = go->GetGUID();
                    break;
            }
        }

        bool SetBossState(uint32 id, EncounterState state)
        {
            if (!InstanceScript::SetBossState(id, state))
                return false;

            switch (id)
            {
                case DATA_VIZIER_ZORLOK:
                {
                    switch (state)
                    {
                        case NOT_STARTED:
                            HandleGameObject(vizierentdoorGuid, true);
                            for (std::vector<ObjectGuid>::const_iterator guid = vizierarenadoorGuids.begin(); guid != vizierarenadoorGuids.end(); guid++)
                                HandleGameObject(*guid, true);
                            break;
                        case DONE:
                            HandleGameObject(vizierentdoorGuid, true);
                            HandleGameObject(vizierexdoorGuid, true);
                            for (std::vector<ObjectGuid>::const_iterator guid = vizierarenadoorGuids.begin(); guid != vizierarenadoorGuids.end(); guid++)
                                HandleGameObject(*guid, true);
                            break;
                        case IN_PROGRESS:
                            HandleGameObject(vizierentdoorGuid, false);
                            for (std::vector<ObjectGuid>::const_iterator guid = vizierarenadoorGuids.begin(); guid != vizierarenadoorGuids.end(); guid++)
                                HandleGameObject(*guid, false);
                            break;
                    }
                    break;
                }
                case DATA_LORD_TAYAK:
                {
                    switch (state)
                    {
                        case NOT_STARTED:
                            HandleGameObject(vizierexdoorGuid, true);
                            break;
                        case DONE:
                            HandleGameObject(vizierexdoorGuid, true);
                            HandleGameObject(tayakexdoorGuid, true);
                            HandleGameObject(garalonentdoorGuid, true);
                            break;
                        case IN_PROGRESS:
                            HandleGameObject(vizierexdoorGuid, false);
                            break;
                    }
                    break;
                }
                case DATA_GARALON:
                {
                    switch (state)
                    {
                        case NOT_STARTED:
                            for (std::vector<ObjectGuid>::const_iterator guid = garaloncdoorGuids.begin(); guid != garaloncdoorGuids.end(); guid++)
                                HandleGameObject(*guid, true);
                            break;
                        case DONE:
                            for (std::vector<ObjectGuid>::const_iterator guid = garaloncdoorGuids.begin(); guid != garaloncdoorGuids.end(); guid++)
                                HandleGameObject(*guid, true);
                        
                            for (std::vector<ObjectGuid>::const_iterator guids = garalonexdoorGuids.begin(); guids != garalonexdoorGuids.end(); guids++)
                                HandleGameObject(*guids, true);
                            break;
                        case IN_PROGRESS:
                            for (std::vector<ObjectGuid>::const_iterator guid = garaloncdoorGuids.begin(); guid != garaloncdoorGuids.end(); guid++)
                                HandleGameObject(*guid, false);
                            break;
                    }
                    break;
                }
            case DATA_MELJARAK:
                {
                    switch (state)
                    {
                        case NOT_STARTED:
                            for (std::vector<ObjectGuid>::const_iterator guids = garalonexdoorGuids.begin(); guids != garalonexdoorGuids.end(); guids++)
                                HandleGameObject(*guids, true);
                            for (std::vector<ObjectGuid>::const_iterator itr = meljarakWeaponRackGUID.begin(); itr != meljarakWeaponRackGUID.end(); itr++)
                                if (GameObject* obj = instance->GetGameObject(*itr))
                                    obj->Respawn();
                            DoRemoveAurasDueToSpellOnPlayers(122220);
                            break;
                        case IN_PROGRESS:
                            for (std::vector<ObjectGuid>::const_iterator guids = garalonexdoorGuids.begin(); guids != garalonexdoorGuids.end(); guids++)
                                HandleGameObject(*guids, false);

                            for (std::vector<ObjectGuid>::const_iterator guid = meljaraksoldiersGuids.begin(); guid != meljaraksoldiersGuids.end(); guid++)
                            {
                                if (Creature* soldier = instance->GetCreature(*guid))
                                {
                                    if (soldier->isAlive() && !soldier->isInCombat())
                                        soldier->AI()->DoZoneInCombat(soldier, 100.0f);
                                }
                            }
                            break;
                        case DONE:
                            DoRemoveAurasDueToSpellOnPlayers(122220);
                            HandleGameObject(meljarakexdoorGuid, true);
                            HandleGameObject(unsokendoorGuid, true);
                            for (std::vector<ObjectGuid>::const_iterator guids = garalonexdoorGuids.begin(); guids != garalonexdoorGuids.end(); guids++)
                                HandleGameObject(*guids, true);
                            break;
                    }
                    break;
                }
                case DATA_UNSOK:
                {
                    switch (state)
                    {
                        case NOT_STARTED:
                            DoRemoveAurasDueToSpellOnPlayers(122370);
                            DoRemoveAurasDueToSpellOnPlayers(122516);
                            HandleGameObject(unsokendoorGuid, true);
                            break;
                        case IN_PROGRESS:
                            HandleGameObject(unsokendoorGuid, false);
                            break;
                        case DONE:
                            DoRemoveAurasDueToSpellOnPlayers(122370);
                            DoRemoveAurasDueToSpellOnPlayers(122516);
                            HandleGameObject(unsokendoorGuid, true);
                            HandleGameObject(unsokexdoorGuid, true);
                        break;
                    }
                    break;
                }
                case DATA_SHEKZEER:
                {
                    switch (state)
                    {
                    case NOT_STARTED:
                        DoRemoveAurasDueToSpellOnPlayers(123713);
                        break;
                    case DONE:
                        HandleGameObject(unsokexdoorGuid, true);
                        DoRemoveAurasDueToSpellOnPlayers(123713);
                        break;
                    case IN_PROGRESS:
                        DoRemoveAurasDueToSpellOnPlayers(123713);
                        HandleGameObject(unsokexdoorGuid, false);
                        break;
                    case FAIL:
                        DoRemoveAurasDueToSpellOnPlayers(123713);
                        break;
                    }
                    break;
                }
            }
            return true;
        }

        void SetData(uint32 type, uint32 data)
        {
            switch (type)
            {
                case DATA_STORM_UNLEASHED:
                    stormUnleashed = data;
                    if (data == NOT_STARTED)
                        DoRemoveAurasDueToSpellOnPlayers(126159);
                    break;
            }
        }

        uint32 GetData(uint32 type) const override
        {
            switch (type)
            {
                case DATA_STORM_UNLEASHED:
                    return stormUnleashed;
            }
            return 0;
        }

        ObjectGuid GetGuidData(uint32 type) const override
        {
            switch (type)
            {
                case NPC_VIZIER_ZORLOK:
                    return zorlokGuid;
                case NPC_GAS_CONTROLLER:
                    return gascontrollerGuid;
                case NPC_LORD_TAYAK:
                    return tayakGuid;
                case NPC_GARALON:
                    return garalonGuid;
                case NPC_MELJARAK:
                    return meljarakGuid;
                    //Meljarak Soldiers
                case NPC_SRATHIK_1:
                    return srathik[0];
                case NPC_SRATHIK_2:
                    return srathik[1];
                case NPC_SRATHIK_3:
                    return srathik[2];
                case NPC_ZARTHIK_1:
                    return zarthik[0];
                case NPC_ZARTHIK_2:
                    return zarthik[1];
                case NPC_ZARTHIK_3:
                    return zarthik[2];
                case NPC_KORTHIK_1:
                    return korthik[0];
                case NPC_KORTHIK_2:
                    return korthik[1];
                case NPC_KORTHIK_3:
                    return korthik[2];
                    //
                case NPC_UNSOK:
                    return unsokGuid;
                case NPC_SHEKZEER:
                    return shekzeerGuid;
                case GO_EMPRESS_COCOON:
                    return empresscocoonGuid;
            }
            return ObjectGuid::Empty;
        }

        uint64 GetData64(uint32 type) override
        {
            ObjectGuid guid = GetGuidData(type);
            if (!guid.IsEmpty())
                return guid.GetCounter();

            return 0;
        }

        void CreatureDies(Creature* cre, Unit* /*killer*/)
        {
            switch (cre->GetEntry())
            {
                case NPC_SRATHIK:
                    for (uint8 n = 0; n < 3; n++)
                        if (srathik[n] == cre->GetGUID())
                            srathik[n] = ObjectGuid::Empty;
                    break;
                case NPC_ZARTHIK:
                    for (uint8 n = 0; n < 3; n++)
                        if (zarthik[n] == cre->GetGUID())
                            zarthik[n] = ObjectGuid::Empty;
                    break;
                case NPC_KORTHIK:
                    for (uint8 n = 0; n < 3; n++)
                        if (korthik[n] == cre->GetGUID())
                            korthik[n] = ObjectGuid::Empty;
                    break;
            }
        }

        void OnCreatureRemove(Creature* cre)
        {
            switch (cre->GetEntry())
            {
                case NPC_SRATHIK:
                    for (uint8 n = 0; n < 3; n++)
                        if (srathik[n] == cre->GetGUID())
                            srathik[n] = ObjectGuid::Empty;
                    break;
                case NPC_ZARTHIK:
                    for (uint8 n = 0; n < 3; n++)
                        if (zarthik[n] == cre->GetGUID())
                            zarthik[n] = ObjectGuid::Empty;
                    break;
                case NPC_KORTHIK:
                    for (uint8 n = 0; n < 3; n++)
                        if (korthik[n] == cre->GetGUID())
                            korthik[n] = ObjectGuid::Empty;
                    break;
            }
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

                if (player->isAlive() && !player->isGameMaster() && !player->HasAura(115877)) // Aura 115877 = Totally Petrified
                    return false;
            }

            return true;
        }

        std::string GetSaveData()
        {
            std::ostringstream saveStream;
            saveStream << GetBossSaveData() << " ";
            return saveStream.str();
        }

        void Load(const char* data)
        {
            std::istringstream loadStream(LoadBossState(data));
            uint32 buff;
            for (uint32 i = 0; i < 7; ++i)
                loadStream >> buff;
        }
    };
};

void AddSC_instance_heart_of_fear()
{
    new instance_heart_of_fear();
}
