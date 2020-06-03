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
SDName: Instance_Magisters_Terrace
SD%Complete: 60
SDComment:  Designed only for Selin Fireheart
SDCategory: Magister's Terrace
EndScriptData */

#include "ScriptMgr.h"
#include "InstanceScript.h"
#include "magisters_terrace.h"

#define MAX_ENCOUNTER      4

/*
0  - Selin Fireheart
1  - Vexallus
2  - Priestess Delrissa
3  - Kael'thas Sunstrider
*/

enum Creatures
{
    NPC_SELIN       = 24723,
    NPC_DELRISSA    = 24560,
    NPC_FELCRYSTALS = 24722
};

enum GameObjects
{
    GO_VEXALLUS_DOOR        = 187896,
    GO_SELIN_DOOR           = 187979,
    GO_SELIN_ENCOUNTER_DOOR = 188065,
    GO_DELRISSA_DOOR        = 187770,
    GO_KAEL_DOOR            = 188064,
    GO_KAEL_STATUE_1        = 188165,
    GO_KAEL_STATUE_2        = 188166,
    GO_ESCAPE_ORB           = 188173
};

class instance_magisters_terrace : public InstanceMapScript
{
public:
    instance_magisters_terrace() : InstanceMapScript("instance_magisters_terrace", 585) { }

    InstanceScript* GetInstanceScript(InstanceMap* map) const
    {
        return new instance_magisters_terrace_InstanceMapScript(map);
    }

    struct instance_magisters_terrace_InstanceMapScript : public InstanceScript
    {
        instance_magisters_terrace_InstanceMapScript(Map* map) : InstanceScript(map) {}

        uint32 Encounter[MAX_ENCOUNTER];
        uint32 DelrissaDeathCount;

        GuidList FelCrystals;
        GuidList::const_iterator CrystalItr;

        ObjectGuid SelinGUID;
        ObjectGuid DelrissaGUID;
        ObjectGuid VexallusDoorGUID;
        ObjectGuid SelinDoorGUID;
        ObjectGuid SelinEncounterDoorGUID;
        ObjectGuid DelrissaDoorGUID;
        ObjectGuid KaelDoorGUID;
        ObjectGuid KaelStatue[2];
        ObjectGuid EscapeOrbGUID;

        bool InitializedItr;

        void Initialize()
        {
            memset(&Encounter, 0, sizeof(Encounter));

            FelCrystals.clear();

            DelrissaDeathCount = 0;

            SelinGUID.Clear();
            DelrissaGUID.Clear();
            VexallusDoorGUID.Clear();
            SelinDoorGUID.Clear();
            SelinEncounterDoorGUID.Clear();
            DelrissaDoorGUID.Clear();
            KaelDoorGUID.Clear();
            KaelStatue[0].Clear();
            KaelStatue[1].Clear();
            EscapeOrbGUID.Clear();

            InitializedItr = false;
        }

        bool IsEncounterInProgress() const
        {
            for (uint8 i = 0; i < MAX_ENCOUNTER; ++i)
                if (Encounter[i] == IN_PROGRESS)
                    return true;
            return false;
        }

        uint32 GetData(uint32 identifier) const
        {
            switch (identifier)
            {
                case DATA_SELIN_EVENT:
                    return Encounter[0];
                case DATA_VEXALLUS_EVENT:
                    return Encounter[1];
                case DATA_DELRISSA_EVENT:
                    return Encounter[2];
                case DATA_KAELTHAS_EVENT:
                    return Encounter[3];
                case DATA_DELRISSA_DEATH_COUNT:
                    return DelrissaDeathCount;
                case DATA_FEL_CRYSTAL_SIZE:
                    return FelCrystals.size();
            }
            return 0;
        }

        void SetData(uint32 identifier, uint32 data)
        {
            switch (identifier)
            {
                case DATA_SELIN_EVENT:
                    Encounter[0] = data;
                    break;
                case DATA_VEXALLUS_EVENT:
                    if (data == DONE)
                        DoUseDoorOrButton(VexallusDoorGUID);
                    Encounter[1] = data;
                    break;
                case DATA_DELRISSA_EVENT:
                    if (data == DONE)
                        DoUseDoorOrButton(DelrissaDoorGUID);
                    if (data == IN_PROGRESS)
                        DelrissaDeathCount = 0;
                    Encounter[2] = data;
                    break;
                case DATA_KAELTHAS_EVENT:
                    Encounter[3] = data;
                    break;
                case DATA_DELRISSA_DEATH_COUNT:
                    if (data == SPECIAL)
                        ++DelrissaDeathCount;
                    else
                        DelrissaDeathCount = 0;
                    break;
            }
        }

        void OnCreatureCreate(Creature* creature)
        {
            switch (creature->GetEntry())
            {
                case NPC_SELIN:
                    SelinGUID = creature->GetGUID();
                    break;
                case NPC_DELRISSA:
                    DelrissaGUID = creature->GetGUID();
                    break;
                case NPC_FELCRYSTALS:
                    FelCrystals.push_back(creature->GetGUID());
                    break;
            }
        }

        void OnGameObjectCreate(GameObject* go)
        {
            switch (go->GetEntry())
            {
                case GO_VEXALLUS_DOOR:
                    VexallusDoorGUID = go->GetGUID();
                    break;
                case GO_SELIN_DOOR:
                    SelinDoorGUID = go->GetGUID();
                    break;
                case GO_SELIN_ENCOUNTER_DOOR:
                    SelinEncounterDoorGUID = go->GetGUID();
                    break;
                case GO_DELRISSA_DOOR:
                    DelrissaDoorGUID = go->GetGUID();
                    break;
                case GO_KAEL_DOOR:
                    KaelDoorGUID = go->GetGUID();
                    break;
                case GO_KAEL_STATUE_1:
                    KaelStatue[0] = go->GetGUID();
                    break;
                case GO_KAEL_STATUE_2:
                    KaelStatue[1] = go->GetGUID();
                    break;
                case GO_ESCAPE_ORB:
                    EscapeOrbGUID = go->GetGUID();
                    break;
            }
        }

        ObjectGuid CrystalItrGet()
        {
            if (!InitializedItr)
            {
                CrystalItr = FelCrystals.begin();
                InitializedItr = true;
            }

            ObjectGuid guid = *CrystalItr;
            ++CrystalItr;
            return guid;
        }

        ObjectGuid GetGuidData(uint32 identifier) const
        {
            switch (identifier)
            {
                case DATA_SELIN:
                    return SelinGUID;
                case DATA_DELRISSA:
                    return DelrissaGUID;
                case DATA_VEXALLUS_DOOR:
                    return VexallusDoorGUID;
                case DATA_SELIN_DOOR:
                    return SelinDoorGUID;
                case DATA_SELIN_ENCOUNTER_DOOR:
                    return SelinEncounterDoorGUID;
                case DATA_DELRISSA_DOOR:
                    return DelrissaDoorGUID;
                case DATA_KAEL_DOOR:
                    return KaelDoorGUID;
                case DATA_KAEL_STATUE_LEFT:
                    return KaelStatue[0];
                case DATA_KAEL_STATUE_RIGHT:
                    return KaelStatue[1];
                case DATA_ESCAPE_ORB:
                    return EscapeOrbGUID;

                case DATA_FEL_CRYSTAL:
                {
                    if (FelCrystals.empty())
                    {
                        TC_LOG_ERROR(LOG_FILTER_TSCR, "Magisters Terrace: No Fel Crystals loaded in Inst Data");
                        return ObjectGuid::Empty;
                    }

                    return const_cast<instance_magisters_terrace_InstanceMapScript*>(this)->CrystalItrGet();
                }
            }
            return ObjectGuid::Empty;
        }
    };

};

void AddSC_instance_magisters_terrace()
{
    new instance_magisters_terrace();
}
