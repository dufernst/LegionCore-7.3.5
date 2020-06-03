/*
 * Copyright (C) 2008-2011 TrinityCore <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "pit_of_saron.h"

// positions for Martin Victus (37591) and Gorkun Ironskull (37592)
Position const SlaveLeaderPos  = {689.7158f, -104.8736f, 513.7360f, 0.0f};
// position for Jaina and Sylvanas
Position const EventLeaderPos2 = {1065.983f, 94.954f, 630.997f, 2.247f};

enum 
{
    NPC_TYRANNUS_INTRO              = 36794,
};

class instance_pit_of_saron : public InstanceMapScript
{
public:
    instance_pit_of_saron() : InstanceMapScript(PoSScriptName, 658) {}

    struct instance_pit_of_saron_InstanceScript : public InstanceScript
    {
        instance_pit_of_saron_InstanceScript(Map* map) : InstanceScript(map)
        {
            SetBossNumber(MAX_ENCOUNTER);
            _teamInInstance = 0;
        }

        void OnPlayerEnter(Player* player) override
        {
            if (!_teamInInstance)
                _teamInInstance = player->GetTeam();
        }

        void OnGameObjectCreate(GameObject* go) override
        {
            switch (go->GetEntry())
            {
                case GO_ICE_WALL:
                    uiIceWall = go->GetGUID();
                    if (GetBossState(DATA_GARFROST) == DONE && GetBossState(DATA_ICK) == DONE)
                        HandleGameObject(uiIceWall, true);
                    break;
            }
        }

        void OnCreatureCreate(Creature* creature) override
        {
            switch (creature->GetEntry())
            {
                case NPC_GARFROST:
                    _garfrostGUID = creature->GetGUID();
                    break;
                case NPC_KRICK:
                    _krickGUID = creature->GetGUID();
                    break;
                case NPC_ICK:
                    _ickGUID = creature->GetGUID();
                    break;
                case NPC_TYRANNUS:
                    _tyrannusGUID = creature->GetGUID();
                    break;
                case NPC_RIMEFANG:
                    _rimefangGUID = creature->GetGUID();
                    break;
                case NPC_TYRANNUS_EVENTS:
                    _tyrannusEventGUID = creature->GetGUID();
                    break;
                case NPC_SYLVANAS_PART1:
                    if (_teamInInstance == ALLIANCE)
                        creature->UpdateEntry(NPC_JAINA_PART1, ALLIANCE);
                    _jainaOrSylvanas1GUID = creature->GetGUID();
                    break;
                case NPC_SYLVANAS_PART2:
                    if (_teamInInstance == ALLIANCE)
                        creature->UpdateEntry(NPC_JAINA_PART2, ALLIANCE);
                    _jainaOrSylvanas2GUID = creature->GetGUID();
                    break;
                case NPC_KILARA:
                    if (_teamInInstance == ALLIANCE)
                       creature->UpdateEntry(NPC_ELANDRA, ALLIANCE);
                    break;
                case NPC_KORALEN:
                    if (_teamInInstance == ALLIANCE)
                       creature->UpdateEntry(NPC_KORLAEN, ALLIANCE);
                    break;
                case NPC_CHAMPION_1_HORDE:
                    if (_teamInInstance == ALLIANCE)
                       creature->UpdateEntry(NPC_CHAMPION_1_ALLIANCE, ALLIANCE);
                    break;
                case NPC_CHAMPION_2_HORDE:
                    if (_teamInInstance == ALLIANCE)
                       creature->UpdateEntry(NPC_CHAMPION_2_ALLIANCE, ALLIANCE);
                    break;
                case NPC_CHAMPION_3_HORDE: // No 3rd set for Alliance?
                    if (_teamInInstance == ALLIANCE)
                       creature->UpdateEntry(NPC_CHAMPION_2_ALLIANCE, ALLIANCE);
                    break;
                case NPC_HORDE_SLAVE_1:
                    if (_teamInInstance == ALLIANCE)
                       creature->UpdateEntry(NPC_ALLIANCE_SLAVE_1, ALLIANCE);
                    break;
                case NPC_HORDE_SLAVE_2:
                    if (_teamInInstance == ALLIANCE)
                       creature->UpdateEntry(NPC_ALLIANCE_SLAVE_2, ALLIANCE);
                    break;
                case NPC_HORDE_SLAVE_3:
                    if (_teamInInstance == ALLIANCE)
                       creature->UpdateEntry(NPC_ALLIANCE_SLAVE_3, ALLIANCE);
                    break;
                case NPC_HORDE_SLAVE_4:
                    if (_teamInInstance == ALLIANCE)
                       creature->UpdateEntry(NPC_ALLIANCE_SLAVE_4, ALLIANCE);
                    break;
                case NPC_FREED_SLAVE_1_HORDE:
                    if (_teamInInstance == ALLIANCE)
                        creature->UpdateEntry(NPC_FREED_SLAVE_1_ALLIANCE, ALLIANCE);
                    break;
                case NPC_FREED_SLAVE_2_HORDE:
                    if (_teamInInstance == ALLIANCE)
                        creature->UpdateEntry(NPC_FREED_SLAVE_2_ALLIANCE, ALLIANCE);
                    break;
                case NPC_FREED_SLAVE_3_HORDE:
                    if (_teamInInstance == ALLIANCE)
                        creature->UpdateEntry(NPC_FREED_SLAVE_3_ALLIANCE, ALLIANCE);
                    break;
                case NPC_RESCUED_SLAVE_HORDE:
                    if (_teamInInstance == ALLIANCE)
                        creature->UpdateEntry(NPC_RESCUED_SLAVE_ALLIANCE, ALLIANCE);
                    break;
                case NPC_MARTIN_VICTUS_1:
                    if (_teamInInstance == ALLIANCE)
                        creature->UpdateEntry(NPC_MARTIN_VICTUS_1, ALLIANCE);
                    break;
                case NPC_MARTIN_VICTUS_END:
                    if (_teamInInstance == ALLIANCE)
                        creature->UpdateEntry(NPC_MARTIN_VICTUS_END, ALLIANCE);
                    break;
                default:
                    break;
            }
        }

        bool SetBossState(uint32 type, EncounterState state) override
        {
            if (!InstanceScript::SetBossState(type, state))
                return false;

            switch (type)
            {
                case DATA_ICK:
                    if (state == DONE)
                    {
                        if (GetBossState(DATA_GARFROST)==DONE)
                            HandleGameObject(uiIceWall,true);
                    }
                    break;
                case DATA_GARFROST:
                    if (state == DONE)
                    {
                        if (Creature* summoner = instance->GetCreature(_garfrostGUID))
                        {
                            if (_teamInInstance == ALLIANCE)
                            {
                                Creature* pMartin = summoner->SummonCreature(NPC_MARTIN_VICTUS_1, 695.46f, -156.31f, 528.061f, 4.77f, TEMPSUMMON_DEAD_DESPAWN, 0);
                                pMartin->GetMotionMaster()->MovePoint(0, pMartin->GetPositionX() + 15, pMartin->GetPositionY() - 5, pMartin->GetPositionZ());
                            }
                            else
                            {
                                Creature* pGorkun = summoner->SummonCreature(NPC_GORKUN_IRONSKULL_1, 695.46f, -156.31f, 528.061f, 4.77f, TEMPSUMMON_DEAD_DESPAWN, 0);
                                pGorkun->GetMotionMaster()->MovePoint(0, pGorkun->GetPositionX() + 15, pGorkun->GetPositionY() - 5, pGorkun->GetPositionZ());
                            }    
                        }
                    }
                    break;
                case DATA_TYRANNUS:
                {
                    if (state == DONE)
                    {
                        if (Creature* summoner = instance->GetCreature(_tyrannusGUID))
                        {
                            if (_teamInInstance == ALLIANCE)
                                summoner->SummonCreature(NPC_JAINA_PART2, EventLeaderPos2, TEMPSUMMON_MANUAL_DESPAWN);
                            else
                                summoner->SummonCreature(NPC_SYLVANAS_PART2, EventLeaderPos2, TEMPSUMMON_MANUAL_DESPAWN);
                        }
                    }
                    if (state == IN_PROGRESS)
                    {
                        if (Creature* summoner = instance->GetCreature(_tyrannusGUID))
                        {
                            if (_teamInInstance == ALLIANCE)
                                summoner->SummonCreature(NPC_MARTIN_VICTUS_END, 1060.955f, 107.274f, 629.424f, 2.084f, TEMPSUMMON_DEAD_DESPAWN, 0);
                            else
                                summoner->SummonCreature(NPC_GORKUN_IRONSKULL_END, 1060.955f, 107.274f, 629.424f, 2.084f, TEMPSUMMON_DEAD_DESPAWN, 0);
                        }
                    }
                    break;
                }
                default:
                    break;
            }
            return true;
        }

        uint32 GetData(uint32 type) const override
        {
            switch (type)
            {
                case DATA_TEAM_IN_INSTANCE:
                    return _teamInInstance;
                default:
                    break;
            }

            return 0;
        }

        ObjectGuid GetGuidData(uint32 type) const override
        {
            switch (type)
            {
                case DATA_GARFROST:
                    return _garfrostGUID;
                case DATA_KRICK:
                    return _krickGUID;
                case DATA_ICK:
                    return _ickGUID;
                case DATA_TYRANNUS:
                    return _tyrannusGUID;
                case DATA_RIMEFANG:
                    return _rimefangGUID;
                case DATA_TYRANNUS_EVENT:
                    return _tyrannusEventGUID;
                case DATA_JAINA_SYLVANAS_1:
                    return _jainaOrSylvanas1GUID;
                case DATA_JAINA_SYLVANAS_2:
                    return _jainaOrSylvanas2GUID;
                default:
                    break;
            }

            return ObjectGuid::Empty;
        }

        bool CheckRequiredBosses(uint32 bossId, uint32 entry, Player const* player = NULL) const override
        {
            switch (bossId)
            {
                case DATA_ICK:
                    if (GetBossState(DATA_GARFROST) != DONE)
                        return false;
                    break;
                default:   
                    break;
            }
            return true;
        }

        std::string GetSaveData() override
        {
            OUT_SAVE_INST_DATA;

            std::ostringstream saveStream;
            saveStream << "P S " << GetBossSaveData();

            OUT_SAVE_INST_DATA_COMPLETE;
            return saveStream.str();
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

            if (dataHead1 == 'P' && dataHead2 == 'S')
            {
                for (uint8 i = 0; i < MAX_ENCOUNTER; ++i)
                {
                    uint32 tmpState;
                    loadStream >> tmpState;
                    if (tmpState == IN_PROGRESS || tmpState > SPECIAL)
                        tmpState = NOT_STARTED;

                    SetBossState(i, EncounterState(tmpState));
                }
            }
            else
                OUT_LOAD_INST_DATA_FAIL;

            OUT_LOAD_INST_DATA_COMPLETE;
        }

        private:
            ObjectGuid _garfrostGUID;
            ObjectGuid _krickGUID;
            ObjectGuid _ickGUID;
            ObjectGuid _tyrannusGUID;
            ObjectGuid _rimefangGUID;

            ObjectGuid _tyrannusEventGUID;
            ObjectGuid _jainaOrSylvanas1GUID;
            ObjectGuid _jainaOrSylvanas2GUID;
            ObjectGuid uiIceWall;
            uint32 _teamInInstance;
    };

    InstanceScript* GetInstanceScript(InstanceMap* map) const override
    {
        return new instance_pit_of_saron_InstanceScript(map);
    }
};

void AddSC_instance_pit_of_saron()
{
    new instance_pit_of_saron();
}
