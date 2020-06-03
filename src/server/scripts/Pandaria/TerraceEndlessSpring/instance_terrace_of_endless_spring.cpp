/*
 * Copyright (C) 2008-20XX JadeCore <http://www.pandashan.com>
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
#include "InstanceScript.h"
#include "VMapFactory.h"
#include "terrace_of_endless_spring.h"

class instance_terrace_of_endless_spring : public InstanceMapScript
{
    public:
        instance_terrace_of_endless_spring() : InstanceMapScript("instance_terrace_of_endless_spring", 996) { }

        InstanceScript* GetInstanceScript(InstanceMap* map) const
        {
            return new instance_terrace_of_endless_spring_InstanceMapScript(map);
        }

        struct instance_terrace_of_endless_spring_InstanceMapScript : public InstanceScript
        {
            instance_terrace_of_endless_spring_InstanceMapScript(Map* map) : InstanceScript(map) { }

            bool ritualOfPurification;
            bool introDone;

            // Protectors of the Endless
            ObjectGuid ancientRegailGuid;
            ObjectGuid ancientAsaniGuid;
            ObjectGuid protectorKaolanGuid;
            ObjectGuid minionOfFearControllerGuid;

            // Tsulong
            ObjectGuid tsulongGuid;

            // Lei Shi
            ObjectGuid leiShiGuid;

            // Sha of Fear
            ObjectGuid shaOfFearGuid;
            ObjectGuid pureLightTerraceGuid;

            // Timers, old school style!
            uint32 tsulongEventTimer;

            // Council's Vortex
            ObjectGuid wallOfCouncilsVortexGuid;
            ObjectGuid councilsVortexGuid;

            // Lei Shi's Vortex
            ObjectGuid wallOfLeiShisVortexGuid;
            ObjectGuid leiShisVortexGuid;

            void Initialize()
            {
                SetBossNumber(DATA_MAX_BOSS_DATA);

                ritualOfPurification        = true;
                introDone                   = false;
            }

            void OnCreatureCreate(Creature* creature)
            {
                switch (creature->GetEntry())
                {
                    case 605866:
                    case NPC_ANCIENT_ASANI:
                        ancientAsaniGuid = creature->GetGUID();
                        break;
                    case 605855:
                    case NPC_ANCIENT_REGAIL:
                        ancientRegailGuid = creature->GetGUID();
                        break;
                    case 605833:
                    case NPC_PROTECTOR_KAOLAN:
                        protectorKaolanGuid = creature->GetGUID();
                        break;
                    case NPC_MINION_OF_FEAR_CONTROLLER:
                        minionOfFearControllerGuid = creature->GetGUID();
                        break;
                    case 624420:
                    case NPC_TSULONG:
                        tsulongGuid = creature->GetGUID();
                        break;
                    case 629833:
                    case NPC_LEI_SHI:
                        leiShiGuid = creature->GetGUID();
                        break;
                    case 609990:
                    case NPC_SHA_OF_FEAR:
                        shaOfFearGuid = creature->GetGUID();
                        break;
                    case NPC_PURE_LIGHT_TERRACE:
                        pureLightTerraceGuid = creature->GetGUID();
                        break;
                    default:
                        break;
                }
            }

            void OnGameObjectCreate(GameObject* go)
            {
                switch (go->GetEntry())
                {
                    case GOB_COUNCILS_VORTEX:
                        councilsVortexGuid = go->GetGUID();
                        break;
                    case GOB_WALL_OF_COUNCILS_VORTEX:
                        wallOfCouncilsVortexGuid = go->GetGUID();
                        break;
                    case GOB_WALL_OF_LEI_SHI:
                        wallOfLeiShisVortexGuid = go->GetGUID();
                        break;
                    case GOB_LEI_SHIS_VORTEX:
                        leiShisVortexGuid = go->GetGUID();
                        break;
                    default:
                        break;
                }
            }

            bool SetBossState(uint32 id, EncounterState state)
            {
                if (!InstanceScript::SetBossState(id, state))
                    return false;

                if (id == DATA_PROTECTORS && state == DONE)
                {
                    if (Creature* c = instance->GetCreature(GetGuidData(NPC_TSULONG)))
                        c->AI()->DoAction(ACTION_START_TSULONG_WAYPOINT);
                }

                return true;
            }

            void SetData(uint32 type, uint32 data)
            {
                switch (type)
                {
                    case SPELL_RITUAL_OF_PURIFICATION:
                        ritualOfPurification = data;
                        break;
                    case INTRO_DONE:
                    {
                        if (data > 0)
                        {
                            Creature* asani = instance->GetCreature(ancientAsaniGuid);
                            if (asani)
                                asani->AI()->DoAction(ACTION_INTRO_FINISHED);

                            Creature* regail = instance->GetCreature(ancientRegailGuid);
                            if (regail)
                                regail->AI()->DoAction(ACTION_INTRO_FINISHED);

                            Creature* kaolan = instance->GetCreature(protectorKaolanGuid);
                            if (kaolan)
                                kaolan->AI()->DoAction(ACTION_INTRO_FINISHED);
                        }
                        introDone = data > 0;
                        break;
                    }
                    default:
                        break;
                }
            }

            uint32 GetData(uint32 type) const override
            {
                switch (type)
                {
                    case SPELL_RITUAL_OF_PURIFICATION:
                        return ritualOfPurification;
                    case INTRO_DONE:
                        return introDone;
                    default:
                        return 0;
                }
            }

            uint64 GetData64(uint32 type) override
            {
                ObjectGuid guid = GetGuidData(type);
                if (!guid.IsEmpty())
                    return guid.GetCounter();

                return 0;
            }

            ObjectGuid GetGuidData(uint32 type) const override
            {
                switch (type)
                {
                    case NPC_ANCIENT_ASANI:
                        return ancientAsaniGuid;
                    case NPC_ANCIENT_REGAIL:
                        return ancientRegailGuid;
                    case NPC_PROTECTOR_KAOLAN:
                        return protectorKaolanGuid;
                    case NPC_MINION_OF_FEAR_CONTROLLER:
                        return minionOfFearControllerGuid;
                    case NPC_TSULONG:
                        return tsulongGuid;
                    case NPC_LEI_SHI:
                        return leiShiGuid;
                    case NPC_SHA_OF_FEAR:
                        return shaOfFearGuid;
                    case NPC_PURE_LIGHT_TERRACE:
                        return pureLightTerraceGuid;
                    case GOB_COUNCILS_VORTEX:
                        return councilsVortexGuid;
                    case GOB_WALL_OF_COUNCILS_VORTEX:
                        return wallOfCouncilsVortexGuid;
                    case GOB_WALL_OF_LEI_SHI:
                        return wallOfLeiShisVortexGuid;
                    case GOB_LEI_SHIS_VORTEX:
                        return leiShisVortexGuid;
                    default:
                        break;
                }

                return ObjectGuid::Empty;
            }

            bool IsWipe() const override
            {
                Map::PlayerList const& PlayerList = instance->GetPlayers();

                if (PlayerList.isEmpty())
                    return true;

                for (Map::PlayerList::const_iterator Itr = PlayerList.begin(); Itr != PlayerList.end(); ++Itr)
                {
                    Player* player = Itr->getSource();

                    if (!player)
                        continue;

                    if (player->isAlive() && !player->isGameMaster() && !player->HasAura(115877)) // Aura 115877 = Totally Petrified
                        return false;
                }

                return true;
            }

            bool CheckRequiredBosses(uint32 bossID, uint32 entry, Player const* player = nullptr) const override
            {
                if (!InstanceScript::CheckRequiredBosses(bossID, entry, player))
                    return false;

                switch (bossID)
                {
                    case DATA_TSULONG:
                    case DATA_LEI_SHI:
                    case DATA_SHA_OF_FEAR:
                        if (GetBossState(bossID - 1) != DONE)
                            return false;
                    default:
                        break;
                }

                return true;
            }
        };
};

void AddSC_instance_terrace_of_endless_spring()
{
    new instance_terrace_of_endless_spring();
}