#include "zulaman.h"

#define MAX_ENCOUNTER 6

static const DoorData doordata[] = 
{
    {GO_AKILZON_EXIT,           DATA_AKILZON,            DOOR_TYPE_ROOM,     BOUNDARY_NONE},
    {GO_HALAZZI_ENTRANCE,       DATA_HALAZZI,            DOOR_TYPE_ROOM,     BOUNDARY_NONE},
    {GO_HALAZZI_EXIT,           DATA_HALAZZI,            DOOR_TYPE_PASSAGE,  BOUNDARY_NONE},
    {GO_MALACRASS_EXIT,         DATA_HEX_LORD_MALACRASS, DOOR_TYPE_PASSAGE,  BOUNDARY_NONE},
    {GO_DAAKARA_EXIT,           DATA_DAAKARA,            DOOR_TYPE_ROOM,     BOUNDARY_NONE},
};

class instance_zulaman : public InstanceMapScript
{
    public:
        instance_zulaman() : InstanceMapScript("instance_zulaman", 568){}
        
        InstanceScript* GetInstanceScript(InstanceMap* map) const
        {
            return new instance_zulaman_InstanceMapScript(map);
        }

        struct instance_zulaman_InstanceMapScript : public InstanceScript
        {
            instance_zulaman_InstanceMapScript(Map* map) : InstanceScript(map) 
            {
                SetBossNumber(MAX_ENCOUNTER);
                LoadDoorData(doordata);

                HexLordGateGUID.Clear();
                MainGateGUID.Clear();
                StrangeGongGUID.Clear();

                QuestTimer = 0;
                QuestMinute = 21;
                uiMainGate = 0;
                uiVendor1 = 0;
                uiVendor2 = 0;

                for (uint8 i = 0; i < 4; ++i)
                {
                    _hostages[i].realGUID.Clear();
                    _hostages[i].corpseGUID.Clear();
                    _hostages[i].lootGUID.Clear();
                    _hostages[i].state = 0;
                }
                archaeologyQuestAura = 0;
            }

            void OnPlayerEnter(Player* player)
            {
                if (archaeologyQuestAura)
                    if (!player->HasAura(archaeologyQuestAura))
                        player->CastSpell(player, archaeologyQuestAura, true);
            }

            void OnCreatureCreate(Creature* pCreature)
            {
                switch (pCreature->GetEntry())
                {
                    case NPC_AMANISHI_TEMPEST:
                        AmanishiTempestGUID = pCreature->GetGUID();
                        break;
                    case NPC_BAKKALZU:
                        _hostages[0].realGUID = pCreature->GetGUID();
                        _hostages[0].state = 0;
                        break;
                    case NPC_BAKKALZU_CORPSE:
                        _hostages[0].corpseGUID = pCreature->GetGUID();
                        pCreature->SetVisible(false);
                        break;
                    case NPC_HAZLEK:
                        _hostages[1].realGUID = pCreature->GetGUID();
                        _hostages[1].state = 0;
                        break;
                    case NPC_HAZLEK_CORPSE:
                        _hostages[1].corpseGUID = pCreature->GetGUID();
                        pCreature->SetVisible(false);
                        break;
                    case NPC_NORKANI:
                        _hostages[2].realGUID = pCreature->GetGUID();
                        _hostages[2].state = 0;
                        break;
                    case NPC_NORKANI_CORPSE:
                        _hostages[2].corpseGUID = pCreature->GetGUID();
                        pCreature->SetVisible(false);
                        break;
                    case NPC_KASHA:
                        //_hostages[3].realGUID = pCreature->GetGUID();
                        //_hostages[3].state = 0;
                        uiKashaGUID = pCreature->GetGUID();
                        break;
                    case NPC_KASHA_CORPSE:
                        _hostages[3].corpseGUID = pCreature->GetGUID();
                        pCreature->SetVisible(false);
                        break;
                    default:
                        break;
                }
            }

            void OnGameObjectCreate(GameObject* pGo)
            {
                switch (pGo->GetEntry())
                {
                    case GO_STRANGE_GONG:
                        StrangeGongGUID = pGo->GetGUID();
                        if (uiMainGate == 1)
                            pGo->SetFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_INTERACT_COND);
                        break;
                    case GO_MAIN_GATE:
                        MainGateGUID = pGo->GetGUID();
                        if (uiMainGate == 1)
                            HandleGameObject(MainGateGUID, true);
                        break;
                    case GO_AKILZON_EXIT:
                        AddDoor(pGo, true);
                        break;
                    case GO_HALAZZI_ENTRANCE:
                        AddDoor(pGo, true);
                        break;
                    case GO_HALAZZI_EXIT:
                        AddDoor(pGo, true);
                        break;
                    case GO_MALACRASS_ENTRANCE:
                        HexLordGateGUID = pGo->GetGUID(); 
                        if (GetBossesDone() >= 2)
                            HandleGameObject(HexLordGateGUID, true, pGo);
                        break;
                    case GO_MALACRASS_EXIT:
                        AddDoor(pGo, true);
                        break;
                    case GO_DAAKARA_EXIT:
                        AddDoor(pGo, true);
                        break;
                    case GO_HAZLEK_TRUNK:
                        _hostages[1].lootGUID = pGo->GetGUID();
                        break;
                    case GO_NORKANI_PACKAGE:
                        _hostages[2].lootGUID = pGo->GetGUID();
                        break;
                    case GO_KASHA_BAG:
                        //_hostages[3].lootGUID = pGo->GetGUID();
                        uiKashaBagGUID = pGo->GetGUID();
                        break;
                    default:
                        break;
                }
            }

            bool SetBossState(uint32 id, EncounterState state)
            {
                if (!InstanceScript::SetBossState(id, state))
                    return false;

                if (state == DONE)
                {
                    switch (id)
                    {
                        case DATA_AKILZON:
                        case DATA_NALORAKK:
                        case DATA_JANALAI:
                        case DATA_HALAZZI:
                        {
                            if (id == DATA_AKILZON) QuestMinute += 15;
                            else if (id == DATA_NALORAKK) QuestMinute += 10;
                            DoUpdateWorldState(static_cast<WorldStates>(3106), QuestMinute);

                            if (_hostages[id].state == 0)
                            {
                                _hostages[id].state = 1;
                                if (_hostages[id].lootGUID)
                                    DoRespawnGameObject(_hostages[id].lootGUID, DAY);
                            }

                            uint8 _bosses = GetBossesDone();

                            if (_bosses >= 2)
                                HandleGameObject(HexLordGateGUID, true);

                            if (_bosses >= 4)
                            {
                                if (QuestMinute)
                                {
                                    DoCastSpellOnPlayers(SPELL_ZULAMAN_ACHIEVEMENT);
                                    if (Creature* kasha = instance->GetCreature(uiKashaGUID))
                                        kasha->AI()->SetData(1, 1); // Run Smart Scripts
                                    QuestMinute = 0;
                                    DoUpdateWorldState(static_cast<WorldStates>(3104), 0);
                                }
                            }
                            break;
                        }
                        default:
                            break;
                    }
                }
                return true;
            }

            void SetData(uint32 type, uint32 data)
            {
                switch (type)
                {
                    case DATA_MAIN_GATE:
                        uiMainGate = data;
                        if (data == 1)
                        {
                            HandleGameObject(MainGateGUID, true);
                            if (GameObject* pGo = instance->GetGameObject(StrangeGongGUID))
                                pGo->SetFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_INTERACT_COND);
                            SaveToDB();
                        }
                        break;
                    case DATA_VENDOR_1:
                        uiVendor1 = data;
                        SaveToDB();
                        break;
                    case DATA_VENDOR_2:
                        uiVendor2 = data;
                        SaveToDB();
                        break;
                    case uint32(-1):
                        archaeologyQuestAura = data;
                        SaveToDB();
                        break;
                    default:
                        break;
                }
            }

            uint32 GetData(uint32 type) const override
            {
                switch (type)
                {
                    case DATA_VENDOR_1:
                        return uiVendor1;
                    case DATA_VENDOR_2:
                        return uiVendor2;
                    default: 
                        return 0;
                }
            }

            void Update(uint32 diff)
            {
                if (QuestMinute)
                {
                    if (QuestTimer <= diff)
                    {
                        QuestMinute--;
                        SaveToDB();
                        QuestTimer += 60000;
                        if (QuestMinute)
                        {
                            DoUpdateWorldState(static_cast<WorldStates>(3104), 1);
                            DoUpdateWorldState(static_cast<WorldStates>(3106), QuestMinute);
                        } 
                        else
                        {
                            DoUpdateWorldState(static_cast<WorldStates>(3104), 0);
                            for (uint8 i = 0; i < 4; ++i)
                            {
                                if (Creature* pHostage = instance->GetCreature(_hostages[i].realGUID))
                                {
                                    if (_hostages[i].state != 0)
                                        continue;

                                    pHostage->DespawnOrUnsummon();
                                    if (Creature* pCorpse = instance->GetCreature(_hostages[i].corpseGUID))
                                        pCorpse->SetVisible(true);
                                    _hostages[i].state = 2;
                                }
                            }
                                    
                        }

                    }
                    QuestTimer -= diff;
                }
            }

            std::string GetSaveData()
            {
                OUT_SAVE_INST_DATA;

                std::ostringstream ss;
                ss << "ZA " << GetBossSaveData() << uiMainGate << " " << QuestMinute << " " << uiVendor1 << " " << uiVendor2 << " " << archaeologyQuestAura;

                OUT_SAVE_INST_DATA_COMPLETE;
                return ss.str();
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

                std::istringstream loadStream(in);
                loadStream >> dataHead1 >> dataHead2;

                if (dataHead1 == 'Z' && dataHead2 == 'A')
                {
                    for (uint8 i = 0; i < MAX_ENCOUNTER; ++i)
                    {
                        uint32 tmpState;
                        loadStream >> tmpState;
                        if (tmpState == IN_PROGRESS || tmpState > SPECIAL)
                            tmpState = NOT_STARTED;
                        SetBossState(i, EncounterState(tmpState));
                    }
                    loadStream >> uiMainGate;
                    loadStream >> QuestMinute;
                    DoUpdateWorldState(static_cast<WorldStates>(3104), QuestMinute);
                    loadStream >> uiVendor1;
                    loadStream >> uiVendor2;
                    loadStream >> archaeologyQuestAura;
                } else OUT_LOAD_INST_DATA_FAIL;

                OUT_LOAD_INST_DATA_COMPLETE;
            }

        private:
            ObjectGuid HexLordGateGUID;
            ObjectGuid MainGateGUID;
            ObjectGuid StrangeGongGUID;
            ObjectGuid AmanishiTempestGUID;
            ObjectGuid uiKashaBagGUID;
            ObjectGuid uiKashaGUID;

            uint32 uiMainGate;
            uint32 uiVendor1;
            uint32 uiVendor2;
            uint32 QuestTimer;
            uint16 QuestMinute;

            uint32 archaeologyQuestAura;

            uint8 GetBossesDone()
            {
                uint8 _bosses = 0;
                for (uint8 i = 0; i < 4; ++i)
                    if (GetBossState(i) == DONE)
                        _bosses++;
                return _bosses;
            }

            struct Hostage
            {
                ObjectGuid realGUID;
                ObjectGuid corpseGUID;
                ObjectGuid lootGUID;
                uint8 state; // 0 - neutral, 1 - saved, 2 - killed
            };

            Hostage _hostages[4];
        };
};

void AddSC_instance_zulaman()
{
    new instance_zulaman();
}

