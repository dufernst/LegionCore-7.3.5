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

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "scarlet_halls.h"

DoorData const doorData[] =
{
    {GO_BRAUN_DOOR,       DATA_BRAUN,         DOOR_TYPE_PASSAGE,    BOUNDARY_NONE},
    {GO_HARLAN_DOOR,      DATA_HARLAN,        DOOR_TYPE_ROOM,       BOUNDARY_NONE},
    {0,                   0,                  DOOR_TYPE_ROOM,       BOUNDARY_NONE}, // END
};

class instance_scarlet_halls : public InstanceMapScript
{
public:
    instance_scarlet_halls() : InstanceMapScript("instance_scarlet_halls", 1001) { }

    InstanceScript* GetInstanceScript(InstanceMap* map) const
    {
        return new instance_scarlet_halls_InstanceMapScript(map);
    }

    struct instance_scarlet_halls_InstanceMapScript : public InstanceScript
    {
        instance_scarlet_halls_InstanceMapScript(Map* map) : InstanceScript(map) 
        {
            SetBossNumber(MAX_ENCOUNTER);
        }

        ObjectGuid BraunGUID;
        ObjectGuid HarlanGUID;
        ObjectGuid KoeglerGUID;

        void Initialize()
        {
            LoadDoorData(doorData);
            BraunGUID.Clear();
            HarlanGUID.Clear();
            KoeglerGUID.Clear();
        }

        bool SetBossState(uint32 type, EncounterState state)
        {
            if (!InstanceScript::SetBossState(type, state))
                return false;
            
            return true;
        }

        void OnGameObjectCreate(GameObject* go)
        {
            switch (go->GetEntry())
            {
                case GO_HARLAN_DOOR:
                case GO_BRAUN_DOOR:
                    AddDoor(go, true);
                    break;
                default:
                    break;
            }
        }

        void OnCreatureCreate(Creature* creature)
        {
            switch (creature->GetEntry())
            {
                case NPC_HOUNDMASTER_BRAUN:    
                    BraunGUID = creature->GetGUID(); 
                    break;
                case NPC_ARMSMASTER_HARLAN:        
                    HarlanGUID = creature->GetGUID(); 
                    break;
                case NPC_FLAMEWEAVER_KOEGLER:     
                    KoeglerGUID = creature->GetGUID();
                    break;
            }
        }

        void SetData(uint32 type, uint32 data)
        {
            /*switch (type)
            {
                default:
                    break;
            }*/
        }

        ObjectGuid GetGuidData(uint32 type) const
        {
            switch (type)
            {
                case NPC_HOUNDMASTER_BRAUN:   
                    return BraunGUID;
                case NPC_ARMSMASTER_HARLAN:              
                    return HarlanGUID;
                case NPC_FLAMEWEAVER_KOEGLER:                  
                    return KoeglerGUID;
            }
            return ObjectGuid::Empty;
        }

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

void AddSC_instance_scarlet_halls()
{
    new instance_scarlet_halls();
}