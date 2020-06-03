/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2006-2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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

/* ScriptData
SDName: Instance_Scholomance
SD%Complete: 100
SDComment:
SDCategory: Scholomance
EndScriptData */

#include "ScriptMgr.h"
#include "scholomance.h"

enum GameObjectId
{
    GO_DOOR       = 211259,
    GO_DOOR2      = 211258,
    GO_DOOR3      = 211256,
    GO_DOOR4      = 211262,
    GO_DOOR5      = 211261,
    GO_DOOR6      = 210771,
    GO_DOOR7      = 211260,
    GO_LAST_DOOR  = 210789
};

class instance_scholomance : public InstanceMapScript
{
public:
    instance_scholomance() : InstanceMapScript("instance_scholomance", 1007) { }

    InstanceScript* GetInstanceScript(InstanceMap* map) const
    {
        return new instance_scholomance_InstanceMapScript(map);
    }

    struct instance_scholomance_InstanceMapScript : public InstanceScript
    {
        instance_scholomance_InstanceMapScript(Map* map) : InstanceScript(map) {}

        //Creature
        ObjectGuid chillheartGuid;
        ObjectGuid barovGuid;
        ObjectGuid rattlegoreGuid;
        ObjectGuid lilianGuid;
        ObjectGuid darkmasterGuid;

        //GameObject
        ObjectGuid doorGuid;
        ObjectGuid door2Guid;
        ObjectGuid door3Guid;
        ObjectGuid door4Guid;
        ObjectGuid door5Guid;
        ObjectGuid door6Guid;
        ObjectGuid door7Guid;
        ObjectGuid lastdoorGuid;

        void Initialize()
        {
            SetBossNumber(5);
            //Creature
            chillheartGuid.Clear();
            barovGuid.Clear();
            rattlegoreGuid.Clear();
            lilianGuid.Clear();
            darkmasterGuid.Clear();
            
            //GameObject
            doorGuid.Clear();
            door2Guid.Clear();
            door3Guid.Clear();
            door4Guid.Clear();
            door5Guid.Clear();
            door6Guid.Clear();
            door7Guid.Clear();
            lastdoorGuid.Clear();
        }

        void OnCreatureCreate(Creature* creature)
        {
            switch (creature->GetEntry())
            {
                case NPC_INSTRUCTOR_CHILLHEART:
                    chillheartGuid = creature->GetGUID();
                    break;
                case NPC_JANDICE_BAROV:
                    barovGuid = creature->GetGUID();
                    break;
                case NPC_RATTLEGORE:
                    rattlegoreGuid = creature->GetGUID();
                    break;
                case NPC_LILIAN_VOSS:
                    lilianGuid = creature->GetGUID();
                    break;
                case NPC_DARKMASTER_GANDLING:
                    darkmasterGuid = creature->GetGUID();
                    break;
            }
        }

        void OnGameObjectCreate(GameObject* go)
        {
            switch (go->GetEntry())
            {
                case GO_DOOR:
                    doorGuid = go->GetGUID(); 
                    break;
                case GO_DOOR2:
                    door2Guid = go->GetGUID(); 
                    break;
                case GO_DOOR3:
                    door3Guid = go->GetGUID(); 
                    break;
                case GO_DOOR4: 
                    door4Guid = go->GetGUID(); 
                    break;
                case GO_DOOR5: 
                    door5Guid = go->GetGUID(); 
                    break;
                case GO_DOOR6:
                    door6Guid = go->GetGUID(); 
                    break;
                case GO_DOOR7:
                    door7Guid = go->GetGUID();
                    break;
                case GO_LAST_DOOR:
                    lastdoorGuid = go->GetGUID();
                    break;
            }
            OpenDoor();
        }

        void OpenDoor()
        {
            HandleGameObject(doorGuid, true);
        }
        
        bool SetBossState(uint32 id, EncounterState state)
        {
            if (!InstanceScript::SetBossState(id, state))
                return false;

            switch (id)
            {
            case DATA_INSTRUCTOR:
                {
                    switch (state)
                    {
                    case NOT_STARTED:
                        HandleGameObject(doorGuid, true);
                        break;
                    case IN_PROGRESS:
                        HandleGameObject(doorGuid, false);
                        break;
                    case DONE:
                        HandleGameObject(doorGuid, true);
                        HandleGameObject(door2Guid, true);
                        break;
                    }
                }
                break;
            case DATA_BAROV:
                {
                    switch (state)
                    {
                    case NOT_STARTED:
                        HandleGameObject(door2Guid, true);
                        break;
                    case IN_PROGRESS: 
                        HandleGameObject(door2Guid, false);
                        break;
                    case DONE:
                        HandleGameObject(door2Guid, true);
                        HandleGameObject(door3Guid, true);
                        break;
                    }
                }
                break;
            case DATA_RATTLEGORE:
                {
                    switch (state)
                    {
                    case NOT_STARTED:
                        HandleGameObject(door3Guid, true);
                        break;
                    case IN_PROGRESS:
                        HandleGameObject(door3Guid, false);
                        break;
                    case DONE:
                        HandleGameObject(door3Guid, true);
                        HandleGameObject(door4Guid, true);
                        HandleGameObject(door5Guid, true);
                        break;
                    }
                }
                break;
            case DATA_LILIAN:
                {
                    switch (state)
                    {
                    case NOT_STARTED:
                        HandleGameObject(door5Guid, true);
                        break;
                    case IN_PROGRESS:
                        HandleGameObject(door5Guid, false);
                        break;
                    case DONE:
                        HandleGameObject(door5Guid, true);
                        HandleGameObject(door6Guid, true);
                        HandleGameObject(door7Guid, true);
                        break;
                    }
                }
                break;
            case DATA_DARKMASTER:
                {
                    switch (state)
                    {
                    case NOT_STARTED:
                    case DONE:
                        HandleGameObject(lastdoorGuid, true);
                        break;
                    case IN_PROGRESS:
                        HandleGameObject(lastdoorGuid, false);
                        break;
                    }
                }                
                break;
            }

            return true;
        }

        ObjectGuid GetGuidData(uint32 type) const
        {
            switch (type)
            {
                case NPC_INSTRUCTOR_CHILLHEART:
                    return chillheartGuid;
                case NPC_JANDICE_BAROV:
                    return barovGuid;
                case NPC_RATTLEGORE:
                    return rattlegoreGuid;
                case NPC_LILIAN_VOSS:
                    return lilianGuid;
                case NPC_DARKMASTER_GANDLING:
                    return darkmasterGuid;
            }

            return ObjectGuid::Empty;
        }

        void SetData(uint32 type, uint32 data){}

        uint32 GetData(uint32 type) const override
        {
            return 0;
        }

        void Update(uint32 diff) 
        {
            // Challenge
            InstanceScript::Update(diff);
        }
    };

};

void AddSC_instance_scholomance()
{
    new instance_scholomance();
}
