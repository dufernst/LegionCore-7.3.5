#include "blackrock_caverns.h"

#define ENCOUNTERS 5

class instance_blackrock_caverns : public InstanceMapScript
{
public:
    instance_blackrock_caverns() : InstanceMapScript("instance_blackrock_caverns", 645) { }

    InstanceScript* GetInstanceScript(InstanceMap* map) const
    {
        return new instance_blackrock_cavernsInstanceMapScript(map);
    }

    struct instance_blackrock_cavernsInstanceMapScript : public InstanceScript
    {
        instance_blackrock_cavernsInstanceMapScript(InstanceMap* map) : InstanceScript(map) { }

        uint32 m_uiEncounter[ENCOUNTERS];

        ObjectGuid uiRomoggGUID;
        ObjectGuid uiCorlaGUID;
        ObjectGuid uiKarshGUID;
        ObjectGuid uiBeautyGUID;
        ObjectGuid uiLordObsidiusGUID;
        ObjectGuid m_uiPortalGUID[4];

        void Initialize()
        {
             uiRomoggGUID.Clear();
             uiCorlaGUID.Clear();
             uiKarshGUID.Clear();
             uiBeautyGUID.Clear();
             uiLordObsidiusGUID.Clear();
             for (int32 i = 0; i < 4; ++i)
                 m_uiPortalGUID[i].Clear();
        }

        void OnCreatureCreate(Creature* pCreature)
        {
            switch(pCreature->GetEntry())
            {
                case NPC_ROMOGG:
                    uiRomoggGUID = pCreature->GetGUID();
                    break;
                case NPC_CORLA:
                    uiCorlaGUID = pCreature->GetGUID();
                    break;
                case NPC_KARSH:
                    uiKarshGUID = pCreature->GetGUID();
                    break;
                case NPC_BEAUTY:
                    uiBeautyGUID = pCreature->GetGUID();
                    break;
                case NPC_ASCENDANT_LORD_OBSIDIUS:
                    uiLordObsidiusGUID = pCreature->GetGUID();
                    break;
            }
        }

        void OnGameObjectCreate(GameObject* pGo)
        {
            switch(pGo->GetEntry())
            {
            case GO_PORTA1:
                {
                m_uiPortalGUID[0] = pGo->GetGUID();
                if (GetData(DATA_ROMOGG) == DONE)
                    HandleGameObject(m_uiPortalGUID[0], true, pGo);
                break;
                }
            case GO_PORTA2:
                {
                m_uiPortalGUID[1] = pGo->GetGUID();
                if (GetData(DATA_CORLA) == DONE)
                    HandleGameObject(m_uiPortalGUID[1], true, pGo);
                break;
                }
            case GO_PORTA3:
                {
                m_uiPortalGUID[2] = pGo->GetGUID();
                if (GetData(DATA_KARSH) == DONE)
                    HandleGameObject(m_uiPortalGUID[2], true, pGo);
                break;
                }
            case GO_PORTA4:
                {
                m_uiPortalGUID[3] = pGo->GetGUID();
                if (GetData(DATA_BEAUTY) == DONE)
                    HandleGameObject(m_uiPortalGUID[3], true, pGo);
                break;
                }
            }
        }

        ObjectGuid GetGuidData(uint32 data) const
        {
            switch(data)
            {
                case DATA_ROMOGG:
                    return uiRomoggGUID;
                case DATA_CORLA:
                    return uiCorlaGUID;
                case DATA_KARSH:
                    return uiKarshGUID;
                case DATA_BEAUTY:
                    return uiBeautyGUID;
                case DATA_ASCENDANT_LORD_OBSIDIUS:
                    return uiLordObsidiusGUID;
            }
            return ObjectGuid::Empty;
        }

        void SetData(uint32 type, uint32 data)
        {
            switch(type)
            {
                case DATA_ROMOGG:
                    m_uiEncounter[0] = data;
                    if (data == DONE)
                        HandleGameObject(m_uiPortalGUID[0], true);
                    break;
                case DATA_CORLA:
                    m_uiEncounter[1] = data;
                    if (data == DONE)
                        HandleGameObject(m_uiPortalGUID[1], true);
                    break;
                case DATA_KARSH:
                    m_uiEncounter[2] = data;
                    if (data == DONE)
                        HandleGameObject(m_uiPortalGUID[2], true);
                    break;
                case DATA_BEAUTY:
                    m_uiEncounter[3] = data;
                    if (data == DONE)
                        HandleGameObject(m_uiPortalGUID[3], true);
                    break;
                case DATA_ASCENDANT_LORD_OBSIDIUS:
                    m_uiEncounter[4] = data;
                    break;
            }
            if (data == DONE)
                SaveToDB();
        }

        uint32 GetData(uint32 type) const override
        {
            switch(type)
            {
                case DATA_ROMOGG:
                    return m_uiEncounter[0];
                case DATA_CORLA:
                    return m_uiEncounter[1];
                case DATA_KARSH:
                    return m_uiEncounter[2];
                case DATA_BEAUTY:
                    return m_uiEncounter[3];
                case DATA_ASCENDANT_LORD_OBSIDIUS:
                    return m_uiEncounter[4];
            }
            return 0;
        }

        std::string GetSaveData()
        {
            OUT_SAVE_INST_DATA;

            std::string str_data;
            std::ostringstream saveStream;
            saveStream << "B C" << " " << m_uiEncounter[0] << " " << m_uiEncounter[1]  << " " << m_uiEncounter[2]  << " " << m_uiEncounter[3] << " " << m_uiEncounter[4];
            str_data = saveStream.str();

            OUT_SAVE_INST_DATA_COMPLETE;
            return str_data;
        }

        void Load(const char* in)
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

            if (dataHead1 == 'B' && dataHead2 == 'C')
            {
                m_uiEncounter[0] = data0;
                m_uiEncounter[1] = data1;
                m_uiEncounter[2] = data2;
                m_uiEncounter[3] = data3;
                m_uiEncounter[4] = data4;

                for(uint8 i=0; i < ENCOUNTERS; ++i)
                    if (m_uiEncounter[i] == IN_PROGRESS)
                        m_uiEncounter[i] = NOT_STARTED;
            }
            else OUT_LOAD_INST_DATA_FAIL;

            OUT_LOAD_INST_DATA_COMPLETE;
        }
    };
};

void AddSC_instance_blackrock_caverns()
{
    new instance_blackrock_caverns();
}
