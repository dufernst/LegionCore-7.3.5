/*
    Dungeon : Iron Docks 93-95
*/

#include "iron_docks.h"

class instance_iron_docks : public InstanceMapScript
{
public:
    instance_iron_docks() : InstanceMapScript("instance_iron_docks", 1195) {}

    InstanceScript* GetInstanceScript(InstanceMap* map) const
    {
        return new instance_iron_docks_InstanceMapScript(map);
    }

    struct instance_iron_docks_InstanceMapScript : public InstanceScript
    {
        instance_iron_docks_InstanceMapScript(Map* map) : InstanceScript(map) {}

        std::list<ObjectGuid> oshirGUIDconteiner;
        std::map<uint32, ObjectGuid> skullocGUIDconteiner;
        std::map<uint32, ObjectGuid> goCage;
        std::map<uint32, std::list<ObjectGuid> > goCageMobs;
        ObjectGuid enforcerGUID[3];
        ObjectGuid oshirGUID;
        uint8 enforCount;
        uint32 m_uiDialogs[5];

        void Initialize() override
        {
            SetBossNumber(MAX_ENCOUNTER);

            enforCount = 0;
            oshirGUID.Clear();
            memset(m_uiDialogs, 0, sizeof(m_uiDialogs));

            for (uint8 i = 0; i < 3; ++i)
                enforcerGUID[i].Clear();
        }

        void OnCreatureCreate(Creature* creature) override
        {
            switch (creature->GetEntry())
            {
                case NPC_MAKOGG_EMBERBLADE:
                    enforcerGUID[0] = creature->GetGUID();
                    break;
                case NPC_NEESA_NOX:
                    enforcerGUID[1] = creature->GetGUID();
                    break;
                case NPC_AHRIOK_DUGRU:
                    enforcerGUID[2] = creature->GetGUID();
                    break;
                case NPC_RYLAK_CAGE:
                case NPC_WOLF_CAGE:
                case NPC_RYLAK_SKYTERROR:
                case NPC_RAVENOUS_WOLF:
                    oshirGUIDconteiner.push_back(creature->GetGUID());
                    break;
                case NPC_OSHIR:
                    oshirGUID = creature->GetGUID();
                    break;
                case NPC_SKULLOC:
                case NPC_KORAMAR:
                case NPC_BLACKHAND_TURRET:
                case NPC_ZOGGOSH:
                    skullocGUIDconteiner[creature->GetEntry()] = creature->GetGUID();
                    break;
            }
            for (uint8 i = 0; i < 19; ++i)
                if(creature->GetDistance(cageSpawn[i]) < 10.0f)
                    goCageMobs[i].push_back(creature->GetGUID());
        }

        void OnGameObjectCreate(GameObject* go) override
        {
            for (uint8 i = 0; i < 19; ++i)
                if(go->GetDistance(cageSpawn[i]) < 1.0f)
                    goCage[i] = go->GetGUID();
        }

        bool SetBossState(uint32 type, EncounterState state) override
        {
            if (!InstanceScript::SetBossState(type, state))
                return false;
            
            switch (type)
            {
                case DATA_G_ENFORCERS:
                {
                    switch (state)
                    {
                        case NOT_STARTED:
                        {
                            for (uint8 i = 0; i < 3; i++)
                                if (Creature* gfor = instance->GetCreature(enforcerGUID[i]))
                                {
                                    gfor->Respawn();
                                    gfor->AI()->EnterEvadeMode();
                                }
                                break;
                        }
                        case DONE:
                            if (Creature* dugu = instance->GetCreature(enforcerGUID[2]))
                                dugu->SetFlag(OBJECT_FIELD_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);
                            if (Creature* skulloc = instance->GetCreature(skullocGUIDconteiner[NPC_SKULLOC]))
                                skulloc->CastSpell(skulloc, SPELL_IRON_DOCKS_BANTER_5, true);
                            break;
                    }
                    break;
                }
                case DATA_OSHIR:
                {
                    switch (state)
                    {
                        case NOT_STARTED:
                        {
                            for (std::list<ObjectGuid>::iterator itr = oshirGUIDconteiner.begin(); itr != oshirGUIDconteiner.end(); ++itr)
                                if (Creature* oshTrash = instance->GetCreature(*itr))
                                {
                                    oshTrash->Respawn();
                                    oshTrash->AI()->EnterEvadeMode();
                                    oshTrash->NearTeleportTo(oshTrash->GetHomePosition());
                                }
                            for (uint8 i = 0; i < 19; ++i)
                                if (GameObject* cage = instance->GetGameObject(goCage[i]))
                                    cage->SetGoState(GO_STATE_READY);
                            break;
                        }
                        case DONE:
                            break;
                    }
                    break;
                }
                case DATA_SKULLOC:
                {
                    switch (state)
                    {
                        case NOT_STARTED:
                            for (std::map<uint32, ObjectGuid>::iterator itr = skullocGUIDconteiner.begin(); itr != skullocGUIDconteiner.end(); ++itr)
                                if (Creature* skullocs = instance->GetCreature(itr->second))
                                    skullocs->AI()->DespawnOnRespawn(FIVE_SECONDS);
                            break;
                        case IN_PROGRESS:
                            for (std::map<uint32, ObjectGuid>::iterator itr = skullocGUIDconteiner.begin(); itr != skullocGUIDconteiner.end(); ++itr)
                                if (Creature* skullocs = instance->GetCreature(itr->second))
                                    skullocs->AI()->DoZoneInCombat(skullocs, 150.0f);
                            break;
                        case DONE:
                            if (Creature* skulloc = instance->GetCreature(skullocGUIDconteiner[NPC_SKULLOC]))
                                skulloc->SetFlag(OBJECT_FIELD_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);

                            DoUpdateAchievementCriteria(CRITERIA_TYPE_COMPLETE_DUNGEON_ENCOUNTER, 1754);
                            break;
                    }
                    break;
                }
            }
            return true;
        }

        void SetData(uint32 type, uint32 data) override
        {
            switch (type)
            {
                case DATA_G_ENFOR_DIED:
                {
                    enforCount = data;
                    if (enforCount == 3)
                        SetBossState(DATA_G_ENFORCERS, DONE);
                    break;
                }
                case DATA_OSHIR_CAGE:
                {
                    if (GameObject* cage = instance->GetGameObject(goCage[data]))
                        cage->SetGoState(GO_STATE_ACTIVE);

                    for (std::list<ObjectGuid>::iterator itr = goCageMobs[data].begin(); itr != goCageMobs[data].end(); ++itr)
                        if (Creature* cageMobs = instance->GetCreature(*itr))
                            cageMobs->AI()->DoZoneInCombat(cageMobs, 100.0f);
                    break;
                }
                case DATA_CAPTAIN_TEXT_1:
                {
                    m_uiDialogs[0] = data;
                    SaveToDB();
                    break;
                }
                case DATA_CAPTAIN_TEXT_3:
                {
                    m_uiDialogs[1] = data;
                    SaveToDB();
                    break;
                }
                case DATA_CAPTAIN_TEXT_4:
                {
                    m_uiDialogs[2] = data;
                    SaveToDB();
                    break;
                }
                case DATA_CAPTAIN_TEXT_5:
                {
                    m_uiDialogs[3] = data;
                    SaveToDB();
                    break;
                }
                case DATA_CAPTAIN_TEXT_6:
                {
                    m_uiDialogs[4] = data;
                    SaveToDB();
                    break;
                }
                default:
                    break;
            }
        }

        ObjectGuid GetGuidData(uint32 type) const override
        {
            switch (type)
            {
                case NPC_MAKOGG_EMBERBLADE:   
                    return enforcerGUID[0];
                case NPC_NEESA_NOX:
                    return enforcerGUID[1];
                case NPC_AHRIOK_DUGRU:
                    return enforcerGUID[2];
                case NPC_OSHIR:
                    return oshirGUID;
            }

            std::map<uint32, ObjectGuid>::const_iterator itr = skullocGUIDconteiner.find(type);
            if (itr != skullocGUIDconteiner.end())
                return itr->second;

            return ObjectGuid::Empty;
        }

        uint32 GetData(uint32 type) const override
        {
            if (type == DATA_G_ENFOR_DIED)
                return enforCount;
            if (type == DATA_CAPTAIN_TEXT_1)
                return m_uiDialogs[0];
            if (type == DATA_CAPTAIN_TEXT_3)
                return m_uiDialogs[1];
            if (type == DATA_CAPTAIN_TEXT_4)
                return m_uiDialogs[2];
            if (type == DATA_CAPTAIN_TEXT_5)
                return m_uiDialogs[3];
            if (type == DATA_CAPTAIN_TEXT_6)
                return m_uiDialogs[4];
            return 0;
        }

        std::string GetDialogSaveData()
        {
            std::ostringstream saveStream;
            for (uint8 i = 0; i < 7; i++)
                saveStream << (uint32)m_uiDialogs[i] << " ";
            return saveStream.str();
        }

        std::string GetSaveData()
        {
            OUT_SAVE_INST_DATA;

            std::string str_data;

            std::ostringstream saveStream;
            saveStream << "I D " << GetDialogSaveData();

            str_data = saveStream.str();

            OUT_SAVE_INST_DATA_COMPLETE;
            return str_data;
        }
        
        void Load(const char* in) override
        {
            if (!in)
            {
                OUT_LOAD_INST_DATA_FAIL;
                return;
            }

            OUT_LOAD_INST_DATA(in);

            char dataHead1, dataHead2;

            std::istringstream loadStream(in);
            loadStream >> dataHead1 >> dataHead2;

            if (dataHead1 == 'I' && dataHead2 == 'D')
            {
                for (uint8 i = 0; i < 5; i++)
                {
                    uint32 tmpDlg;
                    loadStream >> tmpDlg;
                    if (tmpDlg != DONE)
                        tmpDlg = NOT_STARTED;
                    m_uiDialogs[i] = tmpDlg;
                }
            } else OUT_LOAD_INST_DATA_FAIL;

            OUT_LOAD_INST_DATA_COMPLETE;
        }
    };
};

void AddSC_instance_iron_docks()
{
    new instance_iron_docks();
}