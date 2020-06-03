#include "lost_city_of_the_tolvir.h"

enum eScriptText
{
    YELL_FREE = 0
};

class instance_lost_city_of_the_tolvir : public InstanceMapScript
{
    public:
        instance_lost_city_of_the_tolvir() : InstanceMapScript("instance_lost_city_of_the_tolvir", 755) {}

        InstanceScript* GetInstanceScript(InstanceMap* map) const
        {
            return new instance_lost_city_of_the_tolvir_InstanceMapScript(map);
        }

        struct instance_lost_city_of_the_tolvir_InstanceMapScript : public InstanceScript
        {
            instance_lost_city_of_the_tolvir_InstanceMapScript(InstanceMap* map) : InstanceScript(map) { Initialize(); }

            uint32 Encounter[MAX_ENCOUNTER];
            ObjectGuid uiTunnelGUID[6];
            uint8 uiTunnelFlag;
            ObjectGuid uiHusamGUID;
            ObjectGuid uiLockmawGUID;
            ObjectGuid uiAughGUID;
            ObjectGuid uiBarimGUID;
            ObjectGuid uiBlazeGUID;
            ObjectGuid uiHarbingerGUID;
            ObjectGuid uiSiamatGUID;
            ObjectGuid uiSiamatPlatformGUID;
            uint32 uiUpdateTimer;
            bool BosesIsDone;

            void Initialize()
            {
                memset(&Encounter, 0, sizeof(Encounter));
                memset(&uiTunnelGUID, 0, sizeof(uiTunnelGUID));
                uiTunnelFlag = 0;
                uiHusamGUID.Clear();
                uiLockmawGUID.Clear();
                uiAughGUID.Clear();
                uiBarimGUID.Clear();
                uiBlazeGUID.Clear();
                uiSiamatGUID.Clear();
                uiHarbingerGUID.Clear();
                uiSiamatPlatformGUID.Clear();
                uiUpdateTimer = 7000;
                BosesIsDone = false;
            }

            void SiamatFree()
            {
                if (GameObject* platform = instance->GetGameObject(uiSiamatPlatformGUID))
                {
                    platform->RemoveFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_DAMAGED);
                    platform->SetFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_DESTROYED);
                }

                for (int i = 0; i < 6; ++i)
                    if (Creature* tunnel = instance->GetCreature(uiTunnelGUID[i]))
                        tunnel->SetVisible(true);
            }

            void Update(uint32 diff)
            {
                if (BosesIsDone)
                {
                    if (uiUpdateTimer <= diff)
                    {
                        BosesIsDone = false;
                        SiamatFree();

                        if (Creature* siamat = instance->GetCreature(uiSiamatGUID))
                            siamat->AI()->Talk(YELL_FREE);
                    }
                    else
                        uiUpdateTimer -= diff;
                    }
            }

            bool IsEncounterInProgress() const
            {
                for (uint8 i = 0; i < MAX_ENCOUNTER; ++i)
                    if (Encounter[i] == IN_PROGRESS) return true;

                return false;
            }

            void OnGameObjectCreate(GameObject* go)
            {
                if (go->GetEntry() == SIAMAT_PLATFORM)
                {
                    go->setActive(true);
                    uiSiamatPlatformGUID = go->GetGUID();
                }
            }

            void OnCreatureCreate(Creature* creature)
            {
                switch (creature->GetEntry())
                {
                    case BOSS_GENERAL_HUSAM:
                        uiHusamGUID = creature->GetGUID();
                        break;
                    case BOSS_LOCKMAW:
                        uiLockmawGUID = creature->GetGUID();
                        break;
                    case BOSS_AUGH:
                        uiAughGUID = creature->GetGUID();
                        break;
                    case BOSS_HIGH_PROPHET_BARIM:
                        uiBarimGUID = creature->GetGUID();
                        break;
                    case BOSS_SIAMAT:
                        uiSiamatGUID = creature->GetGUID();
                        break;
                    case NPC_WIND_TUNNEL:
                        {
                            creature->SetVisible(false);
                            creature->SetCanFly(true);
                            uiTunnelGUID[uiTunnelFlag] = creature->GetGUID();
                            ++uiTunnelFlag;

                            if (uiTunnelFlag >= 6)
                                uiTunnelFlag = 0;
                        }
                        break;
                }
            }

            ObjectGuid GetGuidData(uint32 type) const
            {
                switch (type)
                {
                    case DATA_GENERAL_HUSAM:      return uiSiamatGUID;
                    case DATA_LOCKMAW:            return uiLockmawGUID;
                    case DATA_AUGH:               return uiAughGUID;
                    case DATA_HIGH_PROPHET_BARIM: return uiBarimGUID;
                    case DATA_BLAZE:              return uiBlazeGUID;
                    case DATA_HARBINGER:          return uiHarbingerGUID;
                    case DATA_SIAMAT:             return uiSiamatGUID;
                }
                return ObjectGuid::Empty;
            }

            uint32 GetData(uint32 type) const override
            {
                return Encounter[type];
            }

            void SetGuidData(uint32 type, ObjectGuid data)
            {
                switch (type)
                {
                    case DATA_HARBINGER:
                        uiHarbingerGUID = data;
                        break;
                    case DATA_BLAZE:
                        uiBlazeGUID = data;
                        break;
                }
            }

            void SetData(uint32 type, uint32 data)
            {
                Encounter[type] = data;

                if (GetData(DATA_GENERAL_HUSAM)==DONE && GetData(DATA_LOCKMAW)==DONE && GetData(DATA_HIGH_PROPHET_BARIM)==DONE)
                    if (Encounter[DATA_SIAMAT] != DONE)
                        BosesIsDone = true;

                if (type == DATA_SIAMAT && data == DONE)
                {
                    SiamatFree();
                }

                if (data == DONE)
                    SaveToDB();
            }

            std::string GetSaveData()
            {
                OUT_SAVE_INST_DATA;

                std::string str_data;

                std::ostringstream saveStream;
                saveStream << "P S " << Encounter[0] << " " << Encounter[1]  << " " << Encounter[2]
                << " " << Encounter[3]  << " " << Encounter[4];

                str_data = saveStream.str();

                OUT_SAVE_INST_DATA_COMPLETE;
                return str_data;
            }

            void Load(char const* in)
            {
                if (!in)
                {
                    OUT_LOAD_INST_DATA_FAIL;
                    return;
                }

                OUT_LOAD_INST_DATA(in);

                char dataHead1, dataHead2;
                uint16 data0, data1, data2, data3, data4;

                std::istringstream loadStream(in);
                loadStream >> dataHead1 >> dataHead2 >> data0 >> data1 >> data2 >> data3 >> data4;

                if (dataHead1 == 'P' && dataHead2 == 'S')
                {
                    Encounter[0] = data0;
                    Encounter[1] = data1;
                    Encounter[2] = data2;
                    Encounter[3] = data3;
                    Encounter[4] = data4;

                    for (uint8 data = 0; data < MAX_ENCOUNTER; ++data)
                    {
                        if (Encounter[data] == IN_PROGRESS)
                            Encounter[data] = NOT_STARTED;

                        SetData(data, Encounter[data]);
                    }
                }
                else
                    OUT_LOAD_INST_DATA_FAIL;

                OUT_LOAD_INST_DATA_COMPLETE;
            }
        };
};

void AddSC_instance_lost_city_of_the_tolvir()
{
    new instance_lost_city_of_the_tolvir();
}
