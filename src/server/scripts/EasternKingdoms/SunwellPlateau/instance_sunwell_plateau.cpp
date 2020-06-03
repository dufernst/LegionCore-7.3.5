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
SDName: Instance_Sunwell_Plateau
SD%Complete: 25
SDComment: VERIFY SCRIPT
SDCategory: Sunwell_Plateau
EndScriptData */

#include "sunwell_plateau.h"

/* Sunwell Plateau:
0 - Kalecgos and Sathrovarr
1 - Brutallus
2 - Felmyst
3 - Eredar Twins (Alythess and Sacrolash)
4 - M'uru
5 - Kil'Jaeden
*/
DoorData const doorData[] =
{
    {188075,       DATA_FELMYST_EVENT,     DOOR_TYPE_PASSAGE,      BOUNDARY_NONE}
};

class instance_sunwell_plateau : public InstanceMapScript
{
public:
    instance_sunwell_plateau() : InstanceMapScript("instance_sunwell_plateau", 580) { }

    InstanceScript* GetInstanceScript(InstanceMap* map) const
    {
        return new instance_sunwell_plateau_InstanceMapScript(map);
    }

    struct instance_sunwell_plateau_InstanceMapScript : public InstanceScript
    {
        instance_sunwell_plateau_InstanceMapScript(Map* map) : InstanceScript(map)
        {
            SetBossNumber(MAX_ENCOUNTER);
        }

        uint32 m_auiEncounter[MAX_ENCOUNTER];

        /** Creatures **/
        ObjectGuid Kalecgos_Dragon;
        ObjectGuid Kalecgos_Human;
        ObjectGuid Sathrovarr;
        ObjectGuid Brutallus;
        ObjectGuid Madrigosa;
        ObjectGuid Felmyst;
        ObjectGuid Alythess;
        ObjectGuid Sacrolash;
        ObjectGuid Muru;
        ObjectGuid KilJaeden;
        ObjectGuid KilJaedenController;
        ObjectGuid Anveena;
        ObjectGuid KalecgosKJ;
        uint32 SpectralPlayers;

        /** GameObjects **/
        ObjectGuid ForceField;                                      // Kalecgos Encounter
        ObjectGuid KalecgosWall[2];                                 // Felmysts Encounter
        ObjectGuid MurusGate[2];                                    // Murus Encounter

        /*** Misc ***/
        uint32 SpectralRealmTimer;
        GuidVector SpectralRealmList;

        void Initialize()
        {
            LoadDoorData(doorData);
            memset(&m_auiEncounter, 0, sizeof(m_auiEncounter));

            SpectralPlayers = 0;

            /*** Misc ***/
            SpectralRealmTimer = 5000;
        }

        bool IsEncounterInProgress() const
        {
            for (uint8 i = 0; i < MAX_ENCOUNTER; ++i)
                if (m_auiEncounter[i] == IN_PROGRESS)
                    return true;

            return false;
        }

        Player* GetPlayerInMap() const
        {
            Map::PlayerList const& players = instance->GetPlayers();

            if (!players.isEmpty())
            {
                for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
                {
                    Player* player = itr->getSource();
                    if (player && !player->HasAura(45839))
                        return player;
                }
            }

            TC_LOG_DEBUG(LOG_FILTER_TSCR, "Instance Sunwell Plateau: GetPlayerInMap, but PlayerList is empty!");
            return NULL;
        }

        void OnCreatureCreate(Creature* creature)
        {
            switch (creature->GetEntry())
            {
                case 24850: Kalecgos_Dragon     = creature->GetGUID(); break;
                case 24891: Kalecgos_Human      = creature->GetGUID(); break;
                case 24892: Sathrovarr          = creature->GetGUID(); break;
                case 24882: Brutallus           = creature->GetGUID(); break;
                case 24895: Madrigosa           = creature->GetGUID(); break;
                case 25038: Felmyst             = creature->GetGUID(); break;
                case 25166: Alythess            = creature->GetGUID(); break;
                case 25165: Sacrolash           = creature->GetGUID(); break;
                case 25741: Muru                = creature->GetGUID(); break;
                case 25315: KilJaeden           = creature->GetGUID(); break;
                case 25608: KilJaedenController = creature->GetGUID(); break;
                case 26046: Anveena             = creature->GetGUID(); break;
                case 25319: KalecgosKJ          = creature->GetGUID(); break;
            }
        }

        void OnGameObjectCreate(GameObject* go)
        {
            switch (go->GetEntry())
            {
                case 188421:
                    ForceField = go->GetGUID();
                    break;
                case 188523:
                    KalecgosWall[0] = go->GetGUID();
                    break;
                case 188524:
                    KalecgosWall[0] = go->GetGUID();
                    break;
                case 188075:
                    AddDoor(go, true);
                    break;
                case 187990:
                    MurusGate[0] = go->GetGUID();
                    break;
                case 188118:
                    if (m_auiEncounter[4] == DONE)
                        HandleGameObject(ObjectGuid::Empty, true, go);
                    MurusGate[1]= go->GetGUID();
                    break;
            }
        }

        bool SetBossState(uint32 id, EncounterState state)
        {
            if (!InstanceScript::SetBossState(id, state))
                return false;

            switch (id)
            {
                case DATA_BRUTALLUS_EVENT:
                    if (state == DONE)
                    {
                        SetData(DATA_BRUTALLUS_EVENT, DONE);
                        if (Creature* f = instance->GetCreature(Felmyst))
                        {
                            f->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                            f->SetVisible(true);
                        }
                    }
                    break;
            }
            return true;
        }

        uint32 GetData(uint32 id) const
        {
            switch (id)
            {
                case DATA_KALECGOS_EVENT:     return m_auiEncounter[0];
                case DATA_BRUTALLUS_EVENT:    return m_auiEncounter[1];
                case DATA_FELMYST_EVENT:      return m_auiEncounter[2];
                case DATA_EREDAR_TWINS_EVENT: return m_auiEncounter[3];
                case DATA_MURU_EVENT:         return m_auiEncounter[4];
                case DATA_KILJAEDEN_EVENT:    return m_auiEncounter[5];
            }
            return 0;
        }

        ObjectGuid GetGuidData(uint32 id) const
        {
            switch (id)
            {
                case DATA_KALECGOS_DRAGON:      return Kalecgos_Dragon;
                case DATA_KALECGOS_HUMAN:       return Kalecgos_Human;
                case DATA_SATHROVARR:           return Sathrovarr;
                case DATA_GO_FORCEFIELD:        return ForceField;
                case DATA_BRUTALLUS:            return Brutallus;
                case DATA_MADRIGOSA:            return Madrigosa;
                case DATA_FELMYST:              return Felmyst;
                case DATA_ALYTHESS:             return Alythess;
                case DATA_SACROLASH:            return Sacrolash;
                case DATA_MURU:                 return Muru;
                case DATA_KILJAEDEN:            return KilJaeden;
                case DATA_KILJAEDEN_CONTROLLER: return KilJaedenController;
                case DATA_ANVEENA:              return Anveena;
                case DATA_KALECGOS_KJ:          return KalecgosKJ;
                case DATA_PLAYER_GUID:
                    if(Player* Target = GetPlayerInMap())
                        return Target->GetGUID();
            }
            return ObjectGuid::Empty;
        }

        void SetData(uint32 id, uint32 data)
        {
            switch (id)
            {
                case DATA_KALECGOS_EVENT:
                    {
                        if (data == NOT_STARTED || data == DONE)
                        {
                            HandleGameObject(ForceField, true);
                            HandleGameObject(KalecgosWall[0], true);
                            HandleGameObject(KalecgosWall[1], true);
                        }
                        else if (data == IN_PROGRESS)
                        {
                            HandleGameObject(ForceField, false);
                            HandleGameObject(KalecgosWall[0], false);
                            HandleGameObject(KalecgosWall[1], false);
                        }
                        m_auiEncounter[0] = data;
                    }
                    break;
                case DATA_BRUTALLUS_EVENT:
                    m_auiEncounter[1] = data;
                    break;
                case DATA_FELMYST_EVENT:
                    m_auiEncounter[2] = data;
                    break;
                case DATA_EREDAR_TWINS_EVENT:
                    m_auiEncounter[3] = data;
                    break;
                case DATA_MURU_EVENT:
                    switch (data)
                    {
                        case DONE:
                            HandleGameObject(MurusGate[0], true);
                            HandleGameObject(MurusGate[1], true);
                            break;
                        case IN_PROGRESS:
                            HandleGameObject(MurusGate[0], false);
                            HandleGameObject(MurusGate[1], false);
                            break;
                        case NOT_STARTED:
                            HandleGameObject(MurusGate[0], true);
                            HandleGameObject(MurusGate[1], false);
                            break;
                    }
                    m_auiEncounter[4] = data; break;
                case DATA_KILJAEDEN_EVENT:     m_auiEncounter[5] = data; break;
            }

            if (data == DONE)
                SaveToDB();
        }

        std::string GetSaveData()
        {
            OUT_SAVE_INST_DATA;
            std::ostringstream stream;
            stream << m_auiEncounter[0] << ' '  << m_auiEncounter[1] << ' '  << m_auiEncounter[2] << ' '  << m_auiEncounter[3] << ' '
                << m_auiEncounter[4] << ' '  << m_auiEncounter[5];

            OUT_SAVE_INST_DATA_COMPLETE;
            return stream.str();
        }

        void Load(const char* in)
        {
            if (!in)
            {
                OUT_LOAD_INST_DATA_FAIL;
                return;
            }

            OUT_LOAD_INST_DATA(in);
            std::istringstream stream(in);
            stream >> m_auiEncounter[0] >> m_auiEncounter[1] >> m_auiEncounter[2] >> m_auiEncounter[3]
                >> m_auiEncounter[4] >> m_auiEncounter[5];
            for (uint8 i = 0; i < MAX_ENCOUNTER; ++i)
                if (m_auiEncounter[i] == IN_PROGRESS)                // Do not load an encounter as "In Progress" - reset it instead.
                    m_auiEncounter[i] = NOT_STARTED;
            OUT_LOAD_INST_DATA_COMPLETE;
        }
    };

};

void AddSC_instance_sunwell_plateau()
{
    new instance_sunwell_plateau();
}
