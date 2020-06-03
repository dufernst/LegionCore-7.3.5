/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
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
#include "DatabaseEnv.h"
#include "Map.h"
#include "Player.h"
#include "GameObject.h"
#include "ChallengeMgr.h"
#include "Creature.h"
#include "CreatureAI.h"
#include "Log.h"
#include "LFGMgr.h"
#include "Group.h"
#include "ScenarioMgr.h"
#include "InstanceSaveMgr.h"
#include "Packets/InstancePackets.h"
#include "Guild.h"
#include "ScriptMgr.h"
#include "EasyJSon.hpp"

BossInfo::BossInfo(): state(TO_BE_DECIDED)
{
}

DoorInfo::DoorInfo(BossInfo* _bossInfo, DoorType _type, BoundaryType _boundary): bossInfo(_bossInfo), type(_type), boundary(_boundary)
{
}

InstanceScript::InstanceScript(Map* map) : initDamageManager(false), _maxInCombatResCount(0), _combatResChargeTime(0), _nextCombatResChargeTime(0)
{
    SetType(ZONE_TYPE_INSTANCE);
    instance = map;
    completedEncounters = 0;
    scenarioStep = 0;
    _challenge = nullptr;
    _inCombatResCount = 0;
    _challengeChestGuids.assign(3, ObjectGuid::Empty);
    _challengeDoorGuids.clear();
    m_DisabledMask = 0;
    _logData = {};
    IsAllowingRelease = false;
}

InstanceScript::~InstanceScript()
{
    if (_challenge)
    {
        _challenge->SetInstanceScript(nullptr);
        _challenge = nullptr;
    }
}

void InstanceScript::DestroyInstance() {}

void InstanceScript::CreateInstance()
{
    initDamageManager = false;
}

void InstanceScript::Load(char const* data)
{
    LoadBossState(data);
}

std::string InstanceScript::GetSaveData()
{
    return GetBossSaveData();
}

void InstanceScript::SaveToDB()
{
    std::string data = GetSaveData();
    if (data.empty())
        return;

    if (InstanceSave* save = sInstanceSaveMgr->GetInstanceSave(instance->GetInstanceId()))
    {
        save->SetData(data);
        save->SetCompletedEncountersMask(GetCompletedEncounterMask());

        for (auto itr = save->m_groupList.begin(); itr != save->m_groupList.end(); ++itr)
            (*itr)->UpdateInstance(save);

        for (auto itr = save->m_playerList.begin(); itr != save->m_playerList.end(); ++itr)
            (*itr)->UpdateInstance(save);
    }
}

void InstanceScript::HandleGameObject(ObjectGuid GUID, bool open, GameObject* go)
{
    if (!go)
        go = instance->GetGameObject(GUID);
    if (go)
        go->SetGoState(open ? GO_STATE_ACTIVE : GO_STATE_READY);
    else
        TC_LOG_DEBUG(LOG_FILTER_TSCR, "InstanceScript: HandleGameObject failed");
}

GameObject* InstanceScript::GetGameObject(uint32 type)
{
    return ObjectAccessor::GetObjectInMap<GameObject>(GetObjectGuid(type), instance, nullptr);
}

GameObject* InstanceScript::GetGameObjectByEntry(uint32 entry)
{
    auto iter = _gameObjectData.find(entry);
    if (iter != _gameObjectData.end())
        return ObjectAccessor::GetObjectInMap<GameObject>(iter->second, instance, nullptr);
    return nullptr;
}

bool InstanceScript::IsEncounterInProgress() const
{
    for (auto const& v : bosses)
        if (v.state == IN_PROGRESS)
            return true;

    return false;
}

void InstanceScript::LoadMinionData(const MinionData* data)
{
    while (data->entry)
    {
        if (data->bossId < bosses.size())
            minions.insert(std::make_pair(data->entry, MinionInfo(&bosses[data->bossId])));

        ++data;
    }
    TC_LOG_DEBUG(LOG_FILTER_TSCR, "InstanceScript::LoadMinionData: " UI64FMTD " minions loaded.", uint64(minions.size()));
}

void InstanceScript::LoadDoorData(const DoorData* data)
{
    while (data->entry)
    {
        if (data->bossId < bosses.size())
            doors.insert(std::make_pair(data->entry, DoorInfo(&bosses[data->bossId], data->type, BoundaryType(data->boundary))));

        ++data;
    }
    TC_LOG_DEBUG(LOG_FILTER_TSCR, "InstanceScript::LoadDoorData: " UI64FMTD " doors loaded.", uint64(doors.size()));
}

void InstanceScript::UpdateMinionState(Creature* minion, EncounterState state)
{
    switch (state)
    {
        case NOT_STARTED:
            if (!minion->isAlive())
                minion->Respawn();
            else if (minion->isInCombat())
                minion->AI()->EnterEvadeMode();
            break;
        case IN_PROGRESS:
            if (!minion->isAlive())
                minion->Respawn();
            else if (!minion->getVictim())
                minion->AI()->DoZoneInCombat();
            break;
        default:
            break;
    }
}

void InstanceScript::UpdateDoorState(GameObject* door)
{
    if(!this || !door)
        return;

    auto lower = doors.lower_bound(door->GetEntry());
    auto upper = doors.upper_bound(door->GetEntry());
    if (lower == upper)
        return;

    bool open = true;
    for (auto itr = lower; itr != upper && open; ++itr)
    {
        switch (itr->second.type)
        {
            case DOOR_TYPE_ROOM:
                open = itr->second.bossInfo->state != IN_PROGRESS;
                break;
            case DOOR_TYPE_PASSAGE:
                open = itr->second.bossInfo->state == DONE;
                break;
            case DOOR_TYPE_SPAWN_HOLE:
                open = itr->second.bossInfo->state == IN_PROGRESS;
                break;
            default:
                break;
        }
    }

    door->SetGoState(open ? GO_STATE_ACTIVE : GO_STATE_READY);
}

void InstanceScript::AddDoor(GameObject* door, bool add)
{
    auto lower = doors.lower_bound(door->GetEntry());
    auto upper = doors.upper_bound(door->GetEntry());
    if (lower == upper)
        return;

    for (auto itr = lower; itr != upper; ++itr)
    {
        DoorInfo const& data = itr->second;

        if (add)
        {
            door->setActive(true);
            data.bossInfo->door[data.type].insert(door);
            switch (data.boundary)
            {
                default:
                case BOUNDARY_NONE:
                    break;
                case BOUNDARY_N:
                case BOUNDARY_S:
                    data.bossInfo->boundary[data.boundary] = door->GetPositionX();
                    break;
                case BOUNDARY_E:
                case BOUNDARY_W:
                    data.bossInfo->boundary[data.boundary] = door->GetPositionY();
                    break;
                case BOUNDARY_NW:
                case BOUNDARY_SE:
                    data.bossInfo->boundary[data.boundary] = door->GetPositionX() + door->GetPositionY();
                    break;
                case BOUNDARY_NE:
                case BOUNDARY_SW:
                    data.bossInfo->boundary[data.boundary] = door->GetPositionX() - door->GetPositionY();
                    break;
            }
        }
        else
            data.bossInfo->door[data.type].erase(door);
    }

    if (add)
        UpdateDoorState(door);
}

void InstanceScript::AddMinion(Creature* minion, bool add)
{
    auto itr = minions.find(minion->GetEntry());
    if (itr == minions.end())
        return;

    if (add)
        itr->second.bossInfo->minion.insert(minion);
    else
        itr->second.bossInfo->minion.erase(minion);
}

bool InstanceScript::SetBossState(uint32 id, EncounterState state)
{
    if (id < bosses.size())
    {
        BossInfo* bossInfo = &bosses[id];
        if (bossInfo->state == TO_BE_DECIDED) // loading
        {
            bossInfo->state = state;
            //TC_LOG_ERROR(LOG_FILTER_GENERAL, "Inialize boss %u state as %u.", id, (uint32)state);
            return false;
        }
        if (bossInfo->state == state)
            return false;

        switch (state)
        {
            case IN_PROGRESS:
                StartCombatResurrection();
                break;
            case NOT_STARTED:
                DoRemovePlayeresCooldownAndDebuff(true);
                break;
            case DONE:
            {
                ResetCombatResurrection();
                DoRemovePlayeresCooldownAndDebuff(false);

                for (auto i : bossInfo->minion)
                    if (i->isWorldBoss() && i->isAlive())
                        return false;

                for (auto const& s : instance->GetPlayers())
                {
                    if (auto player = s.getSource())
                    {
                        if (!player->GetGroup() || !player->GetMap()|| !player->GetGroup()->IsGuildGroup(player->GetGuildGUID(), true, true))
                            continue;

                        if (auto guild = player->GetGuild())
                        {
                            if (instance->IsRaid())
                                guild->CompleteGuildChallenge(ChallengeRaid);
                                //else if (instance->isChallenge()) @TODO
                                //    guild->CompleteGuildChallenge(ChallengeDungeonChallenge);
                            else if (instance->IsDungeon() && !instance->IsEventScenario())
                                guild->CompleteGuildChallenge(ChallengeDungeon);
                        }
                    }
                }
                break;
            }
            default:
                break;
        }

        bossInfo->state = state;
        SaveToDB();

        for (auto & type : bossInfo->door)
            for (auto i : type)
                UpdateDoorState(i);

        for (auto i : bossInfo->minion)
            UpdateMinionState(i, state);

        return true;
    }
    return false;
}

EncounterState InstanceScript::GetBossState(uint32 id) const
{
    return id < bosses.size() ? bosses[id].state : TO_BE_DECIDED;
}

BossBoundaryMap const* InstanceScript::GetBossBoundary(uint32 id) const
{
    return id < bosses.size() ? &bosses[id].boundary : nullptr;
}

std::string InstanceScript::LoadBossState(const char * data)
{
    if (!data)
        return nullptr;

    std::istringstream loadStream(data);
    uint32 buff;
    uint32 bossId = 0;
    for (auto i = bosses.begin(); i != bosses.end(); ++i, ++bossId)
    {
        loadStream >> buff;
        if (buff < TO_BE_DECIDED)
            SetBossState(bossId, static_cast<EncounterState>(buff));
    }
    return loadStream.str();
}

std::string InstanceScript::GetBossSaveData()
{
    std::ostringstream saveStream;
    for (auto & bosse : bosses)
        saveStream << static_cast<uint32>(bosse.state) << ' ';
    return saveStream.str();
}

void InstanceScript::DoUseDoorOrButton(ObjectGuid uiGuid, uint32 uiWithRestoreTime, bool bUseAlternativeState)
{
    if (!this || !instance)
        return;

    if (uiGuid.IsEmpty())
        return;

    if (auto go = instance->GetGameObject(uiGuid))
    {
        if (go->GetGoType() == GAMEOBJECT_TYPE_DOOR || go->GetGoType() == GAMEOBJECT_TYPE_BUTTON)
        {
            if (go->getLootState() == GO_READY)
                go->UseDoorOrButton(uiWithRestoreTime, bUseAlternativeState);
            else if (go->getLootState() == GO_ACTIVATED)
                go->ResetDoorOrButton();
        }
        else
            TC_LOG_ERROR(LOG_FILTER_GENERAL, "SD2: Script call DoUseDoorOrButton, but gameobject entry %u is type %u.", go->GetEntry(), go->GetGoType());
    }
}

void InstanceScript::DoRespawnGameObject(ObjectGuid uiGuid, uint32 uiTimeToDespawn)
{
    if (!this || !instance)
        return;

    if (GameObject* go = instance->GetGameObject(uiGuid))
    {
        //not expect any of these should ever be handled
        if (go->GetGoType() == GAMEOBJECT_TYPE_FISHINGNODE || go->GetGoType() == GAMEOBJECT_TYPE_DOOR ||
            go->GetGoType() == GAMEOBJECT_TYPE_BUTTON || go->GetGoType() == GAMEOBJECT_TYPE_TRAP)
            return;

        if (go->isSpawned())
            return;

        go->SetRespawnTime(uiTimeToDespawn);
    }
}

void InstanceScript::DoUpdateWorldState(WorldStates variableID, uint32 value)
{
    if (!this || !instance)
        return;

    instance->ApplyOnEveryPlayer([&](Player* player)
    {
        player->SendUpdateWorldState(variableID, value);
    });
}

void InstanceScript::DoSendNotifyToInstance(char const* format, ...)
{
    if (!this || !instance)
        return;

    va_list ap;
    va_start(ap, format);
    char buff[1024];
    vsnprintf(buff, 1024, format, ap);
    va_end(ap);

    instance->ApplyOnEveryPlayer([&](Player* player)
    {
        if (auto session = player->GetSession())
            session->SendNotification("%s", buff);
    });
}

void InstanceScript::DoResetAchievementCriteria(CriteriaTypes type, uint64 miscValue1 /*= 0*/, uint64 miscValue2 /*= 0*/, bool evenIfCriteriaComplete /*= false*/)
{
    if (!this || !instance)
        return;

    instance->ApplyOnEveryPlayer([&](Player* player)
    {
        player->GetAchievementMgr()->ResetAchievementCriteria(type, miscValue1, miscValue2, evenIfCriteriaComplete);
    });
}

void InstanceScript::DoCompleteAchievement(uint32 achievement)
{
    if (!this || !instance)
        return;

    auto pAE = sAchievementStore.LookupEntry(achievement);
    if (!pAE)
        return;

    instance->ApplyOnEveryPlayer([&](Player* player)
    {
        player->CompletedAchievement(pAE);
    });
}

void InstanceScript::DoUpdateAchievementCriteria(CriteriaTypes type, uint32 miscValue1 /*= 0*/, uint32 miscValue2 /*= 0*/, uint32 miscValue3 /*= 0*/, Unit* unit /*= nullptr*/)
{
    if (!this || !instance)
        return;

    instance->ApplyOnEveryPlayer([&](Player* player)
    {
        player->UpdateAchievementCriteria(type, miscValue1, miscValue2, miscValue3, unit);
    });
}

void InstanceScript::DoStartTimedAchievement(CriteriaTimedTypes type, uint32 entry)
{
    if (!this || !instance)
        return;

    instance->ApplyOnEveryPlayer([&](Player* player)
    {
        player->GetAchievementMgr()->StartTimedAchievement(type, entry);
    });
}

void InstanceScript::DoStopTimedAchievement(CriteriaTimedTypes type, uint32 entry)
{
    if (!this || !instance)
        return;

    instance->ApplyOnEveryPlayer([&](Player* player)
    {
        player->GetAchievementMgr()->RemoveTimedAchievement(type, entry);
    });
}

void InstanceScript::DoRemoveAurasDueToSpellOnPlayers(uint32 spell)
{
    if (!this || !instance)
        return;

    instance->ApplyOnEveryPlayer([&](Player* player)
    {
        player->RemoveAurasDueToSpell(spell);
        if (auto pet = player->GetPet())
            pet->RemoveAurasDueToSpell(spell);
    });
}

void InstanceScript::DoRemoveAuraFromStackOnPlayers(uint32 spell, ObjectGuid const& casterGUID, AuraRemoveMode mode, uint32 num)
{
    if (!this || !instance)
        return;

    instance->ApplyOnEveryPlayer([&](Player* player)
    {
        player->RemoveAuraFromStack(spell, casterGUID, mode, num);
    });
}

void InstanceScript::DoNearTeleportPlayers(const Position pos, bool casting /*=false*/)
{
    if (!this || !instance)
        return;

    instance->ApplyOnEveryPlayer([&](Player* player)
    {
        player->NearTeleportTo(pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), pos.GetOrientation(), casting);
    });
}

void InstanceScript::DoStartMovie(uint32 movieId)
{
    if (!this || !instance)
        return;

    instance->ApplyOnEveryPlayer([&](Player* player)
    {
        player->SendMovieStart(movieId);
    });
}

void InstanceScript::DoCastSpellOnPlayers(uint32 spell)
{
    if (!this || !instance)
        return;

    instance->ApplyOnEveryPlayer([&](Player* player)
    {
        player->CastSpell(player, spell, true);
    });
}

void InstanceScript::DoRemovePlayeresCooldownAndDebuff(bool wipe)
{
    if (!this || !instance)
        return;

    if (!instance->IsRaid())
        return;

    instance->ApplyOnEveryPlayer([&](Player* player)
    {
        auto cd_list = player->GetSpellCooldowns();
        SpellCooldowns::iterator next;
        for (auto itr = cd_list->begin(); itr != cd_list->end(); itr = next)
        {
            next = itr;
            ++next;

            if (auto spell = sSpellMgr->GetSpellInfo(itr->first))
                if (spell->HasAttribute(SPELL_ATTR10_UNK13))
                    player->RemoveSpellCooldown(itr->first, true);
        }

        auto& Auras = player->GetAppliedAuras();
        for (auto itr = Auras.begin(); itr != Auras.end(); ++itr)
            if (auto aura = itr->second->GetBase()->GetSpellInfo())
                if (aura->GetMisc()->MiscData.Attributes[0] & (SPELL_ATTR0_UNAFFECTED_BY_INVULNERABILITY | SPELL_ATTR0_DEBUFF) && aura->HasAttribute(SPELL_ATTR5_UNK2) || (wipe && aura->HasAttribute(SPELL_ATTR10_UNK13)))
                    player->RemoveAura(itr);
    });
}

void InstanceScript::DoSetAlternatePowerOnPlayers(int32 value)
{
    if (!this || !instance)
        return;

    instance->ApplyOnEveryPlayer([&](Player* player)
    {
        player->SetPower(POWER_ALTERNATE, value);
    });
}

void InstanceScript::RepopPlayersAtGraveyard()
{
    if (!this || !instance)
        return;

    instance->ApplyOnEveryPlayer([&](Player* player)
    {
        player->RepopAtGraveyard();
    });
}

void InstanceScript::DoAddAuraOnPlayers(uint32 spell)
{
    if (!this || !instance)
        return;

    instance->ApplyOnEveryPlayer([&](Player* player)
    {
        player->AddAura(spell, player);
    });
}

bool InstanceScript::ServerAllowsTwoSideGroups()
{
    return sWorld->getBoolConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_GROUP);
}

bool InstanceScript::CheckAchievementCriteriaMeet(uint32 criteria_id, Player const* /*source*/, Unit const* /*target*/ /*= nullptr*/, uint32 /*miscvalue1*/ /*= 0*/)
{
    TC_LOG_ERROR(LOG_FILTER_GENERAL, "Achievement system call InstanceScript::CheckAchievementCriteriaMeet but instance script for map %u not have implementation for achievement criteria %u", instance->GetId(), criteria_id);
    return false;
}

bool InstanceScript::CheckRequiredBosses(uint32 /*bossId*/, uint32 entry, Player const* /*player*/) const
{
    if (m_DisabledMask & (1 << entry))
        return false;

    if (auto aiinstdata = sObjectMgr->GetCreatureAIInstaceData(entry))
        if (aiinstdata->bossidactivete != 0 && GetBossState(aiinstdata->bossidactivete) != DONE)
            return false;

    return true;
}

void InstanceScript::SetCompletedEncountersMask(uint32 newMask)
{
    completedEncounters = newMask;
}

uint32 InstanceScript::GetCompletedEncounterMask() const
{
    return completedEncounters;
}

void InstanceScript::SendEncounterUnit(uint32 type, Unit* unit /*= nullptr*/, uint8 param1 /*= 0*/, uint8 param2 /*= 0*/)
{
    if (!this || !instance)
        return;

    switch (type)
    {
        case ENCOUNTER_FRAME_INSTANCE_START:
        {
            if (!unit)
                return;

            auto encounterId = sObjectMgr->GetDungeonEncounterByCreature(unit->GetEntry());
            if (encounterId)
            {
                WorldPackets::Instance::EncounterStart start;
                start.EncounterID = encounterId;
                start.DifficultyID = unit->GetSpawnMode();
                start.GroupSize = instance->GetPlayersCountExceptGMs();
                start.UnkEncounterDataSize = 0;
                instance->SendToPlayers(start.Write());
            }

            if (instance->IsDungeon())
            {
                WorldPackets::Instance::InstanceEncounterStart eStart;
                eStart.InCombatResCount = _inCombatResCount;
                eStart.MaxInCombatResCount = _maxInCombatResCount;
                eStart.CombatResChargeRecovery = _combatResChargeTime;
                eStart.NextCombatResChargeTime = _nextCombatResChargeTime;
                eStart.InProgress = true;
                instance->SendToPlayers(eStart.Write());

                StartEncounterLogging(encounterId);
            }
            break;
        }
        case ENCOUNTER_FRAME_INSTANCE_END:
        {
            if (!unit)
                return;

            if (uint16 encounterId = sObjectMgr->GetDungeonEncounterByCreature(unit->GetEntry()))
            {
                WorldPackets::Instance::EncounterEnd end;
                end.EncounterID = encounterId;
                end.DifficultyID = unit->GetSpawnMode();
                end.GroupSize = instance->GetPlayersCountExceptGMs();
                end.Success = param1 ? param1 : !unit->isAlive();
                instance->SendToPlayers(end.Write());

                if (param2)
                    instance->SendToPlayers(WorldPackets::Instance::BossKillCredit(encounterId).Write());

                LogCompletedEncounter(true);
            }

            instance->SendToPlayers(WorldPackets::Instance::NullSmsg(SMSG_INSTANCE_ENCOUNTER_END).Write());
            break;
        }
        case ENCOUNTER_FRAME_ENGAGE:
        {
            if (!unit)
                return;

            WorldPackets::Instance::InstanceEncounterEngageUnit engageUnit;
            engageUnit.Unit = unit->GetGUID();
            engageUnit.TargetFramePriority = !param1 ? 1 : param1;
            instance->SendToPlayers(engageUnit.Write());
            instance->m_activeEntry = unit->GetEntry();
            if (uint16 encounterId = sObjectMgr->GetDungeonEncounterByCreature(unit->GetEntry()))
                instance->m_activeEncounter = encounterId;
            break;
        }
        case ENCOUNTER_FRAME_DISENGAGE:
        {
            if (!unit)
                return;

            WorldPackets::Instance::InstanceEncounterDisengageUnit disengageUnit;
            disengageUnit.Unit = unit->GetGUID();
            instance->SendToPlayers(disengageUnit.Write());
            instance->m_activeEntry = 0;
            instance->m_activeEncounter = 0;
            break;
        }
        case ENCOUNTER_FRAME_UPDATE_PRIORITY:
        {
            if (!unit)
                return;

            WorldPackets::Instance::InstanceEncounterChangePriority changePriority;
            changePriority.Unit = unit->GetGUID();
            changePriority.TargetFramePriority = param1;
            instance->SendToPlayers(changePriority.Write());
            break;
        }
        case ENCOUNTER_FRAME_SET_ALLOWING_RELEASE:
        {
            IsAllowingRelease = param1;
            WorldPackets::Instance::InstanceEncounterSetAllowingRelease packet;
            packet.ReleaseAllowed = param1;
            instance->SendToPlayers(packet.Write());
            break;
        }
        /*case ENCOUNTER_FRAME_ADD_TIMER:
        case ENCOUNTER_FRAME_ENABLE_OBJECTIVE:
        case ENCOUNTER_FRAME_DISABLE_OBJECTIVE:
            data << uint8(param1);
            break;
        case ENCOUNTER_FRAME_UPDATE_OBJECTIVE:
            data << uint8(param1);
            data << uint8(param2);
            break;
        case ENCOUNTER_FRAME_UNK7:*/

            //SMSG_INSTANCE_ENCOUNTER_OBJECTIVE_COMPLETE = 0x27F1,
            //SMSG_INSTANCE_ENCOUNTER_OBJECTIVE_START = 0x27F0,
            //SMSG_INSTANCE_ENCOUNTER_OBJECTIVE_UPDATE = 0x27F3,
            //SMSG_INSTANCE_ENCOUNTER_PHASE_SHIFT_CHANGED = 0x27F7,
            //SMSG_INSTANCE_ENCOUNTER_TIMER_START = 0x27EF,
        default:
            break;
    }
}

bool InstanceScript::IsWipe() const
{
    if (!this || !instance)
        return false;

    auto const& PlayerList = instance->GetPlayers();
    if (PlayerList.isEmpty())
        return true;

    for (auto const& Itr : PlayerList)
    {
        auto player = Itr.getSource();
        if (!player)
            continue;

        if (player->isAlive() && !player->isGameMaster())
            return false;
    }

    return true;
}

void InstanceScript::UpdatePhasing()
{
    if (!this || !instance)
        return;

    int8 step = -1;
    if (instance)
        if (uint32 instanceId = instance->GetInstanceId())
            if (Scenario* progress = sScenarioMgr->GetScenario(instanceId))
                step = progress->GetCurrentStep();

    instance->ApplyOnEveryPlayer([&](Player* player)
    {
        if (player->CanContact())
        {
            player->AddDelayedEvent(100, [player, step]() -> void
            {
                PhaseUpdateData phaseUdateData;
                phaseUdateData.AddConditionType(CONDITION_INSTANCE_INFO);
                if (step >= 0)
                    phaseUdateData.AddScenarioUpdate(step);
                player->GetPhaseMgr().NotifyConditionChanged(phaseUdateData);
            });
        }
    });
}

void InstanceScript::SetBossNumber(uint32 number)
{
    //TC_LOG_DEBUG(LOG_FILTER_TSCR, "InstanceScript::SetBossNumber number %u", number);
    if (bosses.size() < number)
        bosses.resize(number);
}

void InstanceScript::BroadcastPacket(WorldPacket const* data) const
{
    if (!this || !instance)
        return;

    instance->ApplyOnEveryPlayer([&](Player* player)
    {
        player->SendDirectMessage(data);
    });
}

void InstanceScript::setScenarioStep(uint32 step)
{
    scenarioStep = step;
}

uint32 InstanceScript::getScenarionStep() const
{
    return scenarioStep;
}

void InstanceScript::UpdateForScript(uint32 diff)
{
    _events.Update(diff);
    m_Functions.Update(diff);

    if (_challenge)
        _challenge->Update(diff);

    if (_nextCombatResChargeTime)
    {
        if (_nextCombatResChargeTime <= diff)
        {
            if (!instance->IsDungeon())
                return;

            if (_inCombatResCount == _maxInCombatResCount)
                return;

            _nextCombatResChargeTime = _combatResChargeTime;
            _inCombatResCount = std::min(++_inCombatResCount, _maxInCombatResCount);

            WorldPackets::Instance::InstanceEncounterGainCombatResurrectionCharge packet;
            packet.CombatResChargeRecovery = _combatResChargeTime;
            packet.InCombatResCount = _inCombatResCount;
            instance->SendToPlayers(packet.Write());
        }
        else
            _nextCombatResChargeTime -= diff;
    }
}

ObjectGuid InstanceScript::GetObjectGuid(uint32 type) const
{
    auto i = _objectGuids.find(type);
    if (i != _objectGuids.end())
        return i->second;

    return ObjectGuid::Empty;
}

ObjectGuid InstanceScript::GetGuidData(uint32 type) const
{
    return GetObjectGuid(type);
}

void InstanceScript::LoadObjectData(ObjectData const* creatureData, ObjectData const* gameObjectData)
{
    if (creatureData)
        LoadObjectData(creatureData, _creatureInfo);

    if (gameObjectData)
        LoadObjectData(gameObjectData, _gameObjectInfo);
}

void InstanceScript::AddDelayedEvent(uint64 timeOffset, std::function<void()>&& function)
{
    m_Functions.AddDelayedEvent(timeOffset, std::move(function));
}

void InstanceScript::SetDisabledBosses(uint32 p_DisableMask)
{
    m_DisabledMask = p_DisableMask;
}

BossInfo* InstanceScript::GetBossInfo(uint32 id)
{
    if (id < bosses.size())
        return &bosses[id];

    return nullptr;
}

void InstanceScript::OnCreatureCreate(Creature* creature)
{
    AddObject(creature, true);
    AddMinion(creature, true);
}

void InstanceScript::OnCreatureRemove(Creature* creature)
{
    AddObject(creature, false);
    AddMinion(creature, false);
}

void InstanceScript::OnGameObjectCreate(GameObject* go)
{
    AddObject(go, true);
    AddDoor(go, true);
}

void InstanceScript::OnGameObjectRemove(GameObject* go)
{
    AddObject(go, false);
    AddDoor(go, false);
}

void InstanceScript::LoadObjectData(ObjectData const* data, ObjectInfoMap& objectInfo)
{
    while (data->entry)
    {
        ASSERT(objectInfo.find(data->entry) == objectInfo.end());
        objectInfo[data->entry] = data->type;
        ++data;
    }
}

void InstanceScript::AddObject(Creature* obj, bool add)
{
    ObjectInfoMap::const_iterator j = _creatureInfo.find(obj->GetEntry());
    if (j != _creatureInfo.end())
        AddObject(obj, j->second, add);

    _creatureData[obj->GetEntry()] = obj->GetGUID();
}

void InstanceScript::AddObject(GameObject* obj, bool add)
{
    ObjectInfoMap::const_iterator j = _gameObjectInfo.find(obj->GetEntry());
    if (j != _gameObjectInfo.end())
        AddObject(obj, j->second, add);

    _gameObjectData[obj->GetEntry()] = obj->GetGUID();
}

void InstanceScript::AddObject(WorldObject* obj, uint32 type, bool add)
{
    if (add)
        _objectGuids[type] = obj->GetGUID();
    else
    {
        auto i = _objectGuids.find(type);
        if (i != _objectGuids.end() && i->second == obj->GetGUID())
            _objectGuids.erase(i);
    }
}

Creature* InstanceScript::GetCreature(uint32 type)
{
    return ObjectAccessor::GetObjectInMap<Creature>(GetObjectGuid(type), instance, nullptr);
}

Creature* InstanceScript::GetCreatureByEntry(uint32 entry)
{
    auto iter = _creatureData.find(entry);
    if (iter != _creatureData.end())
        return ObjectAccessor::GetObjectInMap<Creature>(iter->second, instance, nullptr);

    return nullptr;
}

void InstanceScript::ResetChallengeMode()
{
    if (_challenge)
        _challenge->ResetGo();

    instance->m_respawnChallenge = time(nullptr); // For respawn all mobs
    RepopPlayersAtGraveyard();
    instance->SetSpawnMode(DIFFICULTY_MYTHIC_DUNGEON);
}

void InstanceScript::AddChallengeModeChests(ObjectGuid chestGuid, uint8 chestLevel)
{
    _challengeChestGuids[chestLevel] = chestGuid;
}

ObjectGuid InstanceScript::GetChellngeModeChests(uint8 chestLevel)
{
    return _challengeChestGuids[chestLevel];
}

void InstanceScript::AddChallengeModeDoor(ObjectGuid doorGuid)
{
    _challengeDoorGuids.push_back(doorGuid);
}

void InstanceScript::AddChallengeModeOrb(ObjectGuid orbGuid)
{
    _challengeOrbGuid = orbGuid;
}

void InstanceScript::AddToDamageManager(Creature* creature, uint8 pullNum)
{
    if (!creature || !creature->isAlive())
        return;

    SetPullDamageManager(creature->GetGUID(), pullNum);

    DamageManager manager;
    manager.entry = creature->GetEntry();
    manager.creature = creature;
    manager.guid = creature->GetGUID();

    damageManager[pullNum].push_back(manager);
    initDamageManager = true;
}

bool InstanceScript::CheckDamageManager()
{
    return initDamageManager;
}

void InstanceScript::UpdateDamageManager(ObjectGuid caller, int32 damage, bool heal)
{
    if (!damage)
        return;

    int8 pullNum = GetPullDamageManager(caller);
    if (pullNum < 0)
        return;

    DamageManagerMap::const_iterator itr = damageManager.find(pullNum);
    if (itr == damageManager.end())
        return;

    std::vector<DamageManager> const* manager = &itr->second;
    if (manager->empty())
        return;

    for (auto const& itr2 : *manager)
    {
        // Creature* pull = itr->creature; // If crashed comment this
        if (Creature* pull = instance->GetCreature(itr2.guid)) // If crashed uncomment this
        {
            if (!pull->isAlive() || itr2.guid == caller)
                continue;

            if (!heal && damage >= pull->GetHealth())
                pull->Kill(pull, true);
            else
                pull->SetHealth(pull->GetHealth() - damage);
        }
    }
}

void InstanceScript::SetPullDamageManager(ObjectGuid guid, uint8 pullId)
{
    pullDamageManager[guid] = pullId;
}

int8 InstanceScript::GetPullDamageManager(ObjectGuid guid) const
{
    if (pullDamageManager.empty())
        return -1;

    auto itr = pullDamageManager.find(guid);
    if (itr == pullDamageManager.end())
        return -1;

    return itr->second;
}

void InstanceScript::ResetCombatResurrection()
{
    if (!instance->IsDungeon() || IsChallenge())
        return;

    _inCombatResCount = 0;
    _maxInCombatResCount = 0;
    _combatResChargeTime = 0;
    _nextCombatResChargeTime = 0;
}

void InstanceScript::StartCombatResurrection()
{
    if (!instance->IsDungeon() || IsChallenge())
        return;

    _inCombatResCount = 1;

    uint32 playerCount = instance->GetPlayers().getSize();
    if (!playerCount)
        return;

    float value = 9000.0f / static_cast<float>(playerCount);
    auto timer = uint32(value / 100.0f);

    value -= static_cast<float>(timer) * 100.0f;
    timer *= MINUTE * IN_MILLISECONDS;
    value *= MINUTE / 100.0f * IN_MILLISECONDS;
    timer += uint32(value);

    _maxInCombatResCount = instance->IsRaid() ? 9 : 0;
    _combatResChargeTime = timer;
    _nextCombatResChargeTime = timer;
}

bool InstanceScript::CanUseCombatResurrection() const
{
    if (!instance->IsDungeon())
        return true;

    if (!IsEncounterInProgress() && !IsChallenge())
        return true;

    return _inCombatResCount != 0;
}

void InstanceScript::ConsumeCombatResurrectionCharge()
{
    if (_inCombatResCount == 0)
        return;

    --_inCombatResCount;

    instance->SendToPlayers(WorldPackets::Instance::NullSmsg(SMSG_INSTANCE_ENCOUNTER_IN_COMBAT_RESURRECTION).Write());
}

void InstanceScript::SetChallenge(Challenge* challenge)
{
    _challenge = challenge;

    _inCombatResCount = 1;
    _maxInCombatResCount = 5;
    _combatResChargeTime = 10 * MINUTE * IN_MILLISECONDS;
    _nextCombatResChargeTime = 10 * MINUTE * IN_MILLISECONDS;
}

Challenge* InstanceScript::GetChallenge() const
{
    return _challenge;
}

bool InstanceScript::IsChallenge() const
{
    return _challenge != nullptr;
}

// Redirect query to challenge
void InstanceScript::OnCreatureCreateForScript(Creature* creature)
{
    if (_challenge)
        _challenge->OnCreatureCreateForScript(creature);
}

void InstanceScript::OnCreatureRemoveForScript(Creature* creature)
{
    if (_challenge)
        _challenge->OnCreatureRemoveForScript(creature);
}

void InstanceScript::OnCreatureUpdateDifficulty(Creature* creature)
{
    if (_challenge)
        _challenge->OnCreatureUpdateDifficulty(creature);
}

void InstanceScript::EnterCombatForScript(Creature* creature, Unit* enemy)
{
    if (_challenge)
        _challenge->EnterCombatForScript(creature, enemy);
}

void InstanceScript::CreatureDiesForScript(Creature* creature, Unit* killer)
{
    if (_challenge)
        _challenge->CreatureDiesForScript(creature, killer);
}

void InstanceScript::OnPlayerEnterForScript(Player* player)
{
    if (_challenge)
        _challenge->OnPlayerEnterForScript(player);
}

void InstanceScript::OnPlayerLeaveForScript(Player* player)
{
    if (_challenge)
        _challenge->OnPlayerLeaveForScript(player);
}

void InstanceScript::OnPlayerDiesForScript(Player* player)
{
    if (_challenge)
        _challenge->OnPlayerDiesForScript(player);

    if (_logData.Encounter.is_initialized() && _logData.Encounter->EncounterStarded)
        ++_logData.Encounter->DeadCount;
}

void InstanceScript::OnUnitCharmed(Unit* unit, Unit* charmer)
{
    if (_challenge)
        _challenge->OnUnitCharmed(unit, charmer);
}

void InstanceScript::OnUnitRemoveCharmed(Unit* unit, Unit* charmer)
{
    if (_challenge)
        _challenge->OnUnitRemoveCharmed(unit, charmer);
}

void InstanceScript::OnGameObjectCreateForScript(GameObject* go)
{
    if (_challenge)
        _challenge->OnGameObjectCreateForScript(go);

    // TC_LOG_DEBUG(LOG_FILTER_CHALLENGE, "OnGameObjectCreateForScript GetEntry %u GetGUID %s", go->GetEntry(), go->GetGUID().ToString().c_str());

    if (sChallengeMgr->IsChest(go->GetEntry()))
        _challengeChest = go->GetGUID();

    if (sChallengeMgr->IsDoor(go->GetEntry()))
        AddChallengeModeDoor(go->GetGUID());

    if (go->GetEntry() == ChallengeModeOrb)
        AddChallengeModeOrb(go->GetGUID());
}

void InstanceScript::OnGameObjectRemoveForScript(GameObject* go)
{
    if (_challenge)
        _challenge->OnGameObjectRemoveForScript(go);
}

void InstanceScript::StartEncounterLogging(uint32 encounterId)
{
    _logData = {};
    _logData.Encounter = boost::in_place();

    _logData.Encounter->Expansion = instance->GetEntry()->ExpansionID;
    if (_logData.Encounter->Expansion != CURRENT_EXPANSION)
        return;

    _logData.Encounter->EncounterStarded = true;
    _logData.Encounter->EncounterID = encounterId;
    _logData.Encounter->DifficultyID = instance->GetDifficultyID();
    _logData.Encounter->StartTime = uint32(time(nullptr));

    for (const auto& itr : instance->GetPlayers())
    {
        if (auto player = itr.getSource())
        {
            if (auto group = player->GetGroup())
            {
                if (!player->GetGuild() || !group->IsGuildGroup())
                    continue;

                _logData.Guild = boost::in_place();
                _logData.Guild->GuildID = player->GetGuildId();
                _logData.Guild->GuildFaction = player->GetTeamId();
                _logData.Guild->GuildName = player->GetGuildName();
                break;
            }
        }
    }

    _logData.RealmID = realm.Id.Realm;
    _logData.MapID = instance->GetId();
}

void InstanceScript::LogCompletedEncounter(bool success)
{
    if (instance->IsLfr())
        return;

    _logData.Encounter = boost::in_place();
    _logData.Encounter->CombatDuration = uint32(time(nullptr)) - _logData.Encounter->StartTime;
    _logData.Encounter->EndTime = uint32(time(nullptr));
    _logData.Encounter->Success = success;

    instance->ApplyOnEveryPlayer([&](Player* player)
    {
        LogsSystem::RosterData data;
        data.GuidLow = player->GetGUIDLow();
        data.Name = player->GetName();
        data.Level = player->getLevel();
        data.Class = player->getClass();
        data.SpecID = player->GetSpecializationId();
        data.Role = player->GetSpecializationRole();
        data.ItemLevel = player->GetAverageItemLevelEquipped();
        data.TeamId = player->GetBGTeamId();
        _logData.Rosters.push_back(data);
    });

    sLog->OutPveEncounter(_logData.Serealize().c_str());

    _logData = {};
}
