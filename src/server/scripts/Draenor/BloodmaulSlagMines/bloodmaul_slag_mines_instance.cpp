/*
 * Copyright (C) 2008-2015 TrinityCore <http://www.trinitycore.org/>
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

#include "InstanceScript.h"
#include "bloodmaul_slag_mines.h"

#define MAX_ENCOUNTER 4

class instance_bloodmaul_slag_mines : public InstanceMapScript
{
public:
    instance_bloodmaul_slag_mines() : InstanceMapScript("instance_bloodmaul_slag_mines", 1175) { }

    struct instance_bloodmaul_slag_mines_InstanceScript : public InstanceScript
    {
        instance_bloodmaul_slag_mines_InstanceScript(Map* map) : InstanceScript(map) { }

        void Initialize()
        {
            SetBossNumber(MAX_ENCOUNTER);
            crushtoEvent = 0;
            gogduhEvent = 0;
            cromanProgress = 0;
            for (uint8 i = 0; i < 3; i++)
                boulder[i] = 0;
        }

        void OnCreatureCreate(Creature* creature)
        {
            switch (creature->GetEntry())
            {
                case BOSS_SLAVE_WATCHER_CRUSHTO:
                    crushtoGUID = creature->GetGUID();
                    break;
                case BOSS_FORGEMASTER_GOGDUH:
                    gogduhGUID = creature->GetGUID();
                    break;
                case BOSS_MAGMOLATUS:
                    magmolatusGUID = creature->GetGUID();
                    break;
                case BOSS_ROLTALL:
                    roltallGUID = creature->GetGUID();
                    break;
                case BOSS_GUGROKK:
                    gugrokkGUID = creature->GetGUID();
                    break;
                case NPC_CROMAN:
                    cromanGUID = creature->GetGUID();
                    break;
                default:
                    break;
            }
        }

        void OnGameObjectCreate(GameObject* go)
        {
            switch (go->GetEntry())
            {
                case GO_DEFENCE_WALL:
                    wallGUID = go->GetGUID();
                    break;
                case GO_DRAW_BRIDGE:
                    bridgeGUID = go->GetGUID();
                    break;
                case GO_DEFENCE_WALL_2:
                    wall2GUID = go->GetGUID();
                    break;
                default:
                    break;
            }
        }

        bool SetBossState(uint32 id, EncounterState state)
        {
            if (!InstanceScript::SetBossState(id, state))
                return false;

            switch (id)
            {
                case DATA_MAGMOLATUS:
                    if (GetBossState(DATA_SLAVE_WATCHER_CRUSHTO) == DONE)
                    {
                        //data.SceneID = 345;
                        //data.SceneScriptPackageID = 655;

                        if (GameObject* go = instance->GetGameObject(wallGUID))
                            go->SetGoState(GO_STATE_ACTIVE);

                        if (GameObject* go = instance->GetGameObject(bridgeGUID))
                            go->SetGoState(GO_STATE_ACTIVE);

                        if (Creature* croman = instance->GetCreature(cromanGUID))
                            croman->AI()->DoAction(ACTION_1);
                    }
                    break;
                case DATA_SLAVE_WATCHER_CRUSHTO:
                    if (GetBossState(DATA_MAGMOLATUS) == DONE)
                    {
                        //data.SceneID = 342;
                        //data.SceneScriptPackageID = 652;

                        if (GameObject* go = instance->GetGameObject(wallGUID))
                            go->SetGoState(GO_STATE_ACTIVE);

                        if (GameObject* go = instance->GetGameObject(bridgeGUID))
                            go->SetGoState(GO_STATE_ACTIVE);
                    }
                    break;
                case DATA_ROLTALL:
                    if (state == DONE)
                    {
                        if (GameObject* go = instance->GetGameObject(wall2GUID))
                            go->SetGoState(GO_STATE_ACTIVE);

                        if (Creature* croman = instance->GetCreature(cromanGUID))
                            croman->AI()->DoAction(ACTION_2);
                    }
                    break;
                case DATA_GUGROKK:
                    if (state == DONE)
                    {
                        if (cromanProgress == IN_PROGRESS)
                            if (Creature* croman = instance->GetCreature(cromanGUID))
                                croman->AI()->DoAction(ACTION_2);
                    }
                    break;
                default:
                    break;
            }

            return true;
        }

        void SetData(uint32 type, uint32 data)
        {
            switch (type)
            {
                case DATA_SLAVE_WATCHER_CRUSHTO_EVENT:
                {
                    crushtoEvent = data;
                    if (Creature* crushto = instance->GetCreature(crushtoGUID))
                    {
                        if (crushtoEvent == 3)
                            crushto->AI()->DoAction(ACTION_1);
                        if (crushtoEvent == 5)
                        {
                            crushto->AI()->Talk(1);
                            crushto->SetReactState(REACT_AGGRESSIVE);
                        }
                    }
                    break;
                }
                case DATA_FORGEMASTER_GOGDUH_EVENT:
                    gogduhEvent = data;
                    if (data == DONE)
                    {
                        // boss pre event with  83621 83622 83623 83620 83624 - summon them and let them move by patch + die at last point
                    }
                    break;
                case DATA_CROMAN_PROGRESS:
                    cromanProgress = data;
                    break;
                case DATA_BOULDER_R:
                    boulder[0] = data;
                    break;
                case DATA_BOULDER_L:
                    boulder[1] = data;
                    break;
                case DATA_BOULDER_M:
                    boulder[2] = data;
                    break;
                default:
                    break;
            }
        }

        ObjectGuid GetGuidData(uint32 type) const
        {
            switch (type)
            {
                case DATA_SLAVE_WATCHER_CRUSHTO:
                    return crushtoGUID;
                case DATA_FORGEMASTER_GOGDUH:
                    return gogduhGUID;
                case DATA_MAGMOLATUS:
                    return magmolatusGUID;
                case DATA_ROLTALL:
                    return roltallGUID;
                case DATA_GUGROKK:
                    return gugrokkGUID;
                case DATA_DEFENCE_WALL:
                    return wallGUID;
                case DATA_DEFENCE_WALL_2:
                    return wall2GUID;
                case DATA_DRAW_BRIDGE:
                    return bridgeGUID;
                case DATA_CROMAN_SUMMONER:
                    return summonerGuid;
                case DATA_CROMAN_GUID:
                    return cromanGUID;
                default:
                    return ObjectGuid::Empty;
            }
        }

        void SetGuidData(uint32 type, ObjectGuid data)
        {
            switch (type)
            {
                case DATA_CROMAN_SUMMONER:
                    summonerGuid = data;
                    break;
                case DATA_CROMAN_GUID:
                    cromanGUID = data;
                    break;
                default:
                    break;
            }
        }

    private:
        ObjectGuid crushtoGUID, gogduhGUID, magmolatusGUID, roltallGUID, gugrokkGUID, wallGUID, wall2GUID, bridgeGUID, summonerGuid, cromanGUID;
        uint32 crushtoEvent, gogduhEvent, cromanProgress, boulder[3];
    };

    InstanceScript* GetInstanceScript(InstanceMap* map) const
    {
        return new instance_bloodmaul_slag_mines_InstanceScript(map);
    }
};

void AddSC_instance_bloodmaul_slag_mines()
{
    new instance_bloodmaul_slag_mines();
}
