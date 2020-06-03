#include "AccountMgr.h"
#include "firelands.h"

// areatrigger
// 6929 - quest1
// 6861 - near miniboss 1

#define MAX_ENCOUNTER 7

static const DoorData doordata[] = 
{
    {GO_BRIDGE_OF_RHYOLITH,  DATA_RHYOLITH,  DOOR_TYPE_ROOM,        BOUNDARY_NONE},
    {GO_FIRE_WALL_BALEROC,   DATA_BALEROC,   DOOR_TYPE_ROOM,        BOUNDARY_NONE},
    {GO_RAID_BRIDGE_FORMING, DATA_BALEROC,   DOOR_TYPE_PASSAGE,     BOUNDARY_NONE},
    {GO_STICKY_WEB,          DATA_BETHTILAC, DOOR_TYPE_ROOM,        BOUNDARY_NONE},
    {GO_BRIDGE_OF_RHYOLITH,  DATA_RHYOLITH,  DOOR_TYPE_SPAWN_HOLE,  BOUNDARY_NONE},
    {GO_FIRE_WALL_FANDRAL_1, DATA_STAGHELM,  DOOR_TYPE_PASSAGE,     BOUNDARY_NONE},
    {GO_FIRE_WALL_FANDRAL_2, DATA_STAGHELM,  DOOR_TYPE_PASSAGE,     BOUNDARY_NONE},
    {GO_SULFURON_KEEP,       DATA_RAGNAROS,  DOOR_TYPE_ROOM,        BOUNDARY_NONE},
    {0, 0, DOOR_TYPE_ROOM, BOUNDARY_NONE},
};

class instance_firelands : public InstanceMapScript
{
    public:
        instance_firelands() : InstanceMapScript("instance_firelands", 720) { }

        InstanceScript* GetInstanceScript(InstanceMap* map) const
        {
            return new instance_firelands_InstanceMapScript(map);
        }

        struct instance_firelands_InstanceMapScript : public InstanceScript
        {
            instance_firelands_InstanceMapScript(Map* map) : InstanceScript(map)
            {
                SetBossNumber(MAX_ENCOUNTER);
                LoadDoorData(doordata);
                uiShannoxGUID.Clear();
                uiRiplimbGUID.Clear();
                uiRagefaceGUID.Clear();
                uiRhyolithGUID.Clear();
                uiBalerocGUID.Clear();
                uiRagnarosGUID.Clear();
                uiFirewallBalerockGUID.Clear();
                uiSulfuronBridgeGUID.Clear();
                uiRhyolithHealth = 0;
                uiRagnarosFloor.Clear();
                uiRagnarosCache10.Clear();
                uiRagnarosCache25.Clear();
                uiTimer = 0;
                bEvent = false;
                creaturePortals.clear();
                gameobjectPortals.clear();
            }

            void OnPlayerEnter(Player* pPlayer)
            {
                if (!uiTeamInInstance)
                    uiTeamInInstance = pPlayer->GetTeam();
            }

            void OnCreatureCreate(Creature* pCreature)
            {
                switch (pCreature->GetEntry())
                {
                    case NPC_SHANNOX:
                        uiShannoxGUID = pCreature->GetGUID();
                        break;
                    case NPC_RIPLIMB:
                        uiRiplimbGUID = pCreature->GetGUID();
                        break;
                    case NPC_RAGEFACE:
                        uiRagefaceGUID = pCreature->GetGUID();
                        break;
                    case NPC_BALEROC:
                        uiBalerocGUID = pCreature->GetGUID();
                        pCreature->SetPhaseMask((GetBossState(DATA_SHANNOX)==DONE) && (GetBossState(DATA_RHYOLITH)==DONE) && (GetBossState(DATA_BETHTILAC)==DONE) && (GetBossState(DATA_ALYSRAZOR)==DONE) ? 1 : 2, true);
                        break;
                    case NPC_CIRCLE_OF_THRONES_PORTAL:
                        creaturePortals.push_back(pCreature);
                        if (uiEvent == DONE)
                        {
                            pCreature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                            pCreature->SetFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP | UNIT_NPC_FLAG_SPELLCLICK);
                        }
                        break;
                    case NPC_RHYOLITH:
                        uiRhyolithGUID = pCreature->GetGUID();
                        break;
                    case NPC_RAGNAROS:
                        uiRagnarosGUID = pCreature->GetGUID();
                        break;
                    default:
                        break;
                }
            }

            void OnGameObjectCreate(GameObject* pGo)
            {
                switch (pGo->GetEntry())
                {
                    case GO_FIRE_WALL_BALEROC:
                        uiFirewallBalerockGUID = pGo->GetGUID();
                        HandleGameObject(ObjectGuid::Empty, (GetBossState(DATA_SHANNOX)==DONE) && (GetBossState(DATA_RHYOLITH)==DONE) && (GetBossState(DATA_BETHTILAC)==DONE) && (GetBossState(DATA_ALYSRAZOR)==DONE), pGo);
                        break;
                    case GO_STICKY_WEB:
                    case GO_RAID_BRIDGE_FORMING:
                    case GO_BRIDGE_OF_RHYOLITH:
                    case GO_FIRE_WALL_FANDRAL_1:
                    case GO_FIRE_WALL_FANDRAL_2:
                    case GO_SULFURON_KEEP:
                        AddDoor(pGo, true);
                        break;
                    case GO_SULFURON_BRIDGE:
                        uiSulfuronBridgeGUID = pGo->GetGUID();
                        if (GetBossState(DATA_BALEROC)==DONE)
                            pGo->SetDestructibleState(GO_DESTRUCTIBLE_DESTROYED);
                        break;
                    case GO_RAGNAROS_FLOOR:
                        uiRagnarosFloor = pGo->GetGUID();
                        break;
                    case GO_CIRCLE_OF_THORNS_PORTAL3:
                        gameobjectPortals.push_back(pGo);
                        if (uiEvent == DONE)
                            HandleGameObject(pGo->GetGUID(), true, pGo);
                        break;
                    case GO_CACHE_OF_THE_FIRELORD_10:
                        uiRagnarosCache10 = pGo->GetGUID();
                        break;
                    case GO_CACHE_OF_THE_FIRELORD_25:
                        uiRagnarosCache25 = pGo->GetGUID();
                        break;
                }
            }

            void SetData(uint32 type, uint32 data)
            {
                if (type == DATA_RHYOLITH_HEALTH_SHARED)
                    uiRhyolithHealth = data;
                else if (type == DATA_EVENT)
                {
                    uiEvent = data;
                    if (uiEvent == DONE)
                    {
                        if (!gameobjectPortals.empty())
                        {
                            for (std::list<GameObject*>::const_iterator itr = gameobjectPortals.begin(); itr != gameobjectPortals.end(); ++itr)
                                if (GameObject* pGo = (*itr)->ToGameObject())
                                    HandleGameObject(pGo->GetGUID(), true, pGo);
                        }

                        if (!creaturePortals.empty())
                        {
                            for (std::list<Creature*>::const_iterator itr = creaturePortals.begin(); itr != creaturePortals.end(); ++itr)
                                if (Creature* pCreature = (*itr)->ToCreature())
                                {
                                    pCreature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                                    pCreature->SetFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP | UNIT_NPC_FLAG_SPELLCLICK);
                                }
                        }

                        SaveToDB();
                    }
                }
            }

            uint32 GetData(uint32 type) const override
            {
                if (type == DATA_RHYOLITH_HEALTH_SHARED)
                    return uiRhyolithHealth;
                else if (type == DATA_EVENT)
                    return uiEvent;
                return 0;
            }

            ObjectGuid GetGuidData(uint32 type) const
            {
                switch (type)
                {
                    case DATA_SHANNOX: return uiShannoxGUID;
                    case DATA_RIPLIMB: return uiRiplimbGUID;
                    case DATA_RAGEFACE: return uiRagefaceGUID;
                    case DATA_RHYOLITH: return uiRhyolithGUID;
                    case DATA_RAGNAROS: return uiRagnarosGUID;
                    case DATA_RAGNAROS_FLOOR: return uiRagnarosFloor;
                    case DATA_RAGNAROS_CACHE_10: return uiRagnarosCache10;
                    case DATA_RAGNAROS_CACHE_25: return uiRagnarosCache25;
                    default: return ObjectGuid::Empty;
                }
                return ObjectGuid::Empty;
            }

            bool SetBossState(uint32 type, EncounterState state)
            {
                if (!InstanceScript::SetBossState(type, state))
                    return false;

                bool balerocAvailable = (GetBossState(DATA_SHANNOX)==DONE) && (GetBossState(DATA_RHYOLITH)==DONE) && (GetBossState(DATA_BETHTILAC)==DONE) && (GetBossState(DATA_ALYSRAZOR)==DONE);

                switch (type)
                {
                case DATA_BALEROC:
                    if (state == DONE)
                        if (GameObject* obj = instance->GetGameObject(uiSulfuronBridgeGUID))
                                obj->SetDestructibleState(GO_DESTRUCTIBLE_DESTROYED);
                    if (state == IN_PROGRESS)
                        HandleGameObject(uiFirewallBalerockGUID, false);
                    else
                        HandleGameObject(uiFirewallBalerockGUID, balerocAvailable);
                    break;
                case DATA_SHANNOX:
                case DATA_RHYOLITH:
                case DATA_BETHTILAC:
                case DATA_ALYSRAZOR:
                    if (uiFirewallBalerockGUID)
                        HandleGameObject(uiFirewallBalerockGUID, balerocAvailable);
                    if (balerocAvailable)
                        if (Creature* baleroc = instance->GetCreature(uiBalerocGUID))
                            baleroc->SetPhaseMask(1, true);
                    break;
                }

                return true;
            }

            bool CheckRequiredBosses(uint32 bossId, uint32 entry, Player const* player = NULL) const
            {
                if (player && player->GetSession() && AccountMgr::IsGMAccount(player->GetSession()->GetSecurity()))
                    return true;

                switch (bossId)
                {
                    case DATA_RAGNAROS:
                        if (GetBossState(DATA_STAGHELM) != DONE)
                            return false;
                        break;
                    case DATA_STAGHELM:
                        if (GetBossState(DATA_BALEROC) != DONE)
                            return false;
                        break;
                    case DATA_BALEROC:
                        if (GetBossState(DATA_SHANNOX) != DONE)
                            return false;
                        if (GetBossState(DATA_ALYSRAZOR) != DONE)
                            return false;
                        if (GetBossState(DATA_BETHTILAC) != DONE)
                            return false;
                        if (GetBossState(DATA_RHYOLITH) != DONE)
                            return false;
                        break;
                    default:
                        break;
                }

                return true;
            }

            void ProcessEvent(WorldObject* /*source*/, uint32 eventId)
            {
                switch (eventId)
                {
                    case EVENT_PORTALS:
                        if ((uiEvent == DONE) || bEvent)
                            return;
                        bEvent = true;
                        uiTimer = 7000;
                        if (!creaturePortals.empty())
                            for (std::list<Creature*>::const_iterator itr = creaturePortals.begin(); itr != creaturePortals.end(); ++itr)
                                if (Creature* pCreature = (*itr)->ToCreature())
                                    pCreature->CastSpell(pCreature, SPELL_LEGENDARY_PORTAL_OPENING);
                        break;
                }
            }

            void Update(uint32 diff)
            {
                if (bEvent)
                {
                    if (uiTimer <= diff)
                    {
                        bEvent = false;
                        SetData(DATA_EVENT, DONE);
                    }
                    else
                        uiTimer -= diff;
                }
            }

            std::string GetSaveData()
            {
                OUT_SAVE_INST_DATA;

                std::string str_data;

                std::ostringstream saveStream;
                saveStream << "F L " << GetBossSaveData() << uiEvent << ' ';

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

                std::istringstream loadStream(in);
                loadStream >> dataHead1 >> dataHead2;

                if (dataHead1 == 'F' && dataHead2 == 'L')
                {
                    for (uint8 i = 0; i < MAX_ENCOUNTER; ++i)
                    {
                        uint32 tmpState;
                        loadStream >> tmpState;
                        if (tmpState == IN_PROGRESS || tmpState > SPECIAL)
                            tmpState = NOT_STARTED;
                        SetBossState(i, EncounterState(tmpState));
                    }

                    uint32 tempEvent = 0;
                    loadStream >> tempEvent;
                    uiEvent = (tempEvent != DONE ? 0 : DONE);

                } else OUT_LOAD_INST_DATA_FAIL;

                OUT_LOAD_INST_DATA_COMPLETE;
            }

            private:
                uint32 uiTeamInInstance;
                uint32 uiRhyolithHealth;
                uint32 uiEvent;
                uint32 uiTimer;
                bool bEvent;
                ObjectGuid uiShannoxGUID;
                ObjectGuid uiRiplimbGUID;
                ObjectGuid uiRagefaceGUID;
                ObjectGuid uiBalerocGUID;
                ObjectGuid uiRagnarosGUID;
                ObjectGuid uiRhyolithGUID;
                ObjectGuid uiFirewallBalerockGUID;
                ObjectGuid uiSulfuronBridgeGUID;
                ObjectGuid uiRagnarosFloor;
                ObjectGuid uiRagnarosCache10;
                ObjectGuid uiRagnarosCache25;
                ObjectGuid uiRagnarosCache10h;
                ObjectGuid uiRagnarosCache25h;
                std::list<GameObject*> gameobjectPortals;
                std::list<Creature*> creaturePortals;
        };
};

void AddSC_instance_firelands()
{
    new instance_firelands();
}
