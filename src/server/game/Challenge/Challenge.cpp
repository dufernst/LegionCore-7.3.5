/*
 * Copyright (C) 2008-2014 TrinityCore <http://www.trinitycore.org/>
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

#include "Challenge.h"
#include "ChallengeMgr.h"
#include "ChallengeModePackets.h"
#include "Chat.h"
#include "GroupMgr.h"
#include "DatabaseEnv.h"
#include "InstancePackets.h"
#include "Map.h"
#include "MiscPackets.h"
#include "WorldStatePackets.h"

Challenge::Challenge(Map* map, Player* player, uint32 instanceID, Scenario* scenario) : InstanceScript(map), _instanceScript(nullptr), _challengeEntry(nullptr), _isKeyDepleted(false), _scenario(scenario)
{
    if (!player)
    {
        _canRun = false;
        return;
    }

    _checkStart = true;
    _canRun = true;
    _creator = player->GetGUID();
    _instanceID = instanceID;
    _challengeTimer = 0;
    _affixQuakingTimer = 0;
    _deathCount = 0;
    _complete = false;
    _run = false;
    _item = nullptr;

    ASSERT(map);
    _map = map;
    _mapID = _map->GetId();

    if (Group* group = player->GetGroup())
    {
        group->m_challengeInstanceID = _instanceID;
        m_ownerGuid = group->m_challengeOwner;
        m_itemGuid = group->m_challengeItem;
        _challengeEntry = group->m_challengeEntry;
        m_gguid = group->GetGUID();

        if (!m_itemGuid)
        {
            _canRun = false;
            ChatHandler(player).PSendSysMessage("Error: Key not found.");
            return;
        }

        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
            if (Player* member = itr->getSource())
                _challengers.insert(member->GetGUID());

        _affixes = group->m_affixes;
        _challengeLevel = group->m_challengeLevel;
    }
    else
    {
        m_ownerGuid = player->GetGUID();
        if (Item* item = player->GetItemByEntry(138019))
            m_itemGuid = item->GetGUID();
        else
        {
            _canRun = false;
            ChatHandler(player).PSendSysMessage("Error: Key not found.");
            return;
        }
        _challengeLevel = player->m_challengeKeyInfo.Level;
        _challengeEntry = player->m_challengeKeyInfo.challengeEntry;

        _affixes.fill(0);
        if (_challengeLevel > 3)
            _affixes[0] = player->m_challengeKeyInfo.Affix;
        if (_challengeLevel > 6)
            _affixes[1] = player->m_challengeKeyInfo.Affix1;
        if (_challengeLevel > 9)
            _affixes[2] = player->m_challengeKeyInfo.Affix2;
    }

    if (!_challengeEntry)
    {
        ChatHandler(player).PSendSysMessage("Error: Is not a challenge map.");
        _canRun = false;
        return;
    }

    _challengers.insert(_creator);

    _rewardLevel = CHALLENGE_NOT_IN_TIMER;

    for (auto const& affix : _affixes)
        _affixesTest.set(affix);

    for (uint8 i = 0; i < CHALLENGE_TIMER_LEVEL_3; i++)
        _chestTimers[i] = _challengeEntry->CriteriaCount[i];
}

Challenge::~Challenge()
{
    if (InstanceScript* script = GetInstanceScript())
        script->SetChallenge(nullptr);

    _instanceScript = nullptr;
}

void Challenge::OnPlayerEnterForScript(Player* player)
{
    if (!player)
        return;

    player->CastSpell(player, ChallengersBurden, true);
}

void Challenge::OnPlayerLeaveForScript(Player* player)
{
    if (!player)
        return;

    player->RemoveAura(ChallengersBurden);
}

void Challenge::OnPlayerDiesForScript(Player* /*player*/)
{
    ++_deathCount;
    ModChallengeTimer(Seconds(5).count());

    SendStartElapsedTimer();

    WorldPackets::ChallengeMode::ChallengeModeUpdateDeathCount packet;
    packet.DeathCount = _deathCount;
    BroadcastPacket(packet.Write());
}

void Challenge::OnCreatureCreateForScript(Creature* creature)
{
    if (!creature || creature->isTrigger() || creature->IsControlledByPlayer() || creature->GetCreatureType() == CREATURE_TYPE_CRITTER)
        return;

    Unit* owner = creature->GetAnyOwner();
    if (owner && owner->IsPlayer())
        return;

    creature->AddAura(ChallengersMight, creature);
}

void Challenge::OnCreatureUpdateDifficulty(Creature* creature)
{
    if (!creature || creature->isTrigger() || creature->IsControlledByPlayer() || creature->GetCreatureType() == CREATURE_TYPE_CRITTER)
        return;

    Unit* owner = creature->GetAnyOwner();
    if (owner && owner->IsPlayer())
        return;

    creature->AddAura(ChallengersMight, creature);
}

void Challenge::OnCreatureRemoveForScript(Creature* /*creature*/) { }

void Challenge::EnterCombatForScript(Creature* creature, Unit* /*enemy*/)
{
    if (!creature || creature->isTrigger() || creature->IsControlledByPlayer())
        return;

    Unit* owner = creature->GetAnyOwner();
    if (owner && owner->IsPlayer())
        return;

    if (!creature->HasAura(ChallengersMight))
        creature->AddAura(ChallengersMight, creature);
}

void Challenge::CreatureDiesForScript(Creature* creature, Unit* /*killer*/)
{
    if (!creature || creature->isTrigger() || creature->IsControlledByPlayer() || !creature->IsHostileToPlayers() || creature->GetCreatureType() == CREATURE_TYPE_CRITTER)
        return;

    if (creature->GetAnyOwner() || creature->IsOnVehicle())
        return;

    if (HasAffix(Affixes::Bolstering) && !creature->IsAffixDisabled(Affixes::Bolstering))
        creature->CastSpell(creature, ChallengerBolstering, true);

    if (HasAffix(Affixes::Sanguine) && !creature->IsAffixDisabled(Affixes::Sanguine))
        creature->CastSpell(creature, ChallengerSanguine, true);

    if (HasAffix(Affixes::Bursting) && !creature->IsAffixDisabled(Affixes::Bursting))
        creature->CastSpell(creature, ChallengerBursting, true);
}

void Challenge::OnUnitCharmed(Unit* unit, Unit* /*charmer*/)
{
    if (!unit || !unit->ToCreature())
        return;

    unit->RemoveAura(ChallengerBolstering);
    unit->RemoveAura(ChallengerRaging);
    unit->RemoveAura(ChallengersMight);
    unit->RemoveAura(207850); // Bond of Strength
}

void Challenge::OnUnitRemoveCharmed(Unit* unit, Unit* /*charmer*/)
{
    if (!unit || !unit->ToCreature())
        return;

    unit->AddAura(ChallengersMight, unit);
    // unit->RemoveAura(ChallengerBolstering);
}

void Challenge::Update(uint32 diff)
{
    m_Functions.Update(diff);

    if (_complete) // Stop update if complete
        return;

    if (!_run && _checkStart)
    {
        if (CanStart())
        {
            Start();
            SummonWall(nullptr);
        }
        return;
    }

    _challengeTimer += diff;

    if (_affixQuakingTimer)
    {
        if (_affixQuakingTimer <= diff)
        {
            _map->ApplyOnEveryPlayer([&](Player* plr)
            {
                if (plr->CanContact() && !plr->HasAura(ChallengerQuake) && !plr->HasFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_NO_ACTIONS) && !plr->IsOnVehicle())
                    plr->AddAura(ChallengerQuake, plr);
            });
            _affixQuakingTimer = 20000;
        }
        else
            _affixQuakingTimer -= diff;
    }

    if (!_isKeyDepleted && GetChallengeTimer() > _challengeEntry->CriteriaCount[0])
    {
        _isKeyDepleted = true;

        if (Group* group = sGroupMgr->GetGroupByGUID(m_gguid))
        {
            group->m_challengeEntry = nullptr;
            group->m_challengeLevel = 0;
            group->m_affixes.fill(0);
            group->SetDungeonDifficultyID(DIFFICULTY_MYTHIC_DUNGEON);
            group->m_challengeInstanceID = 0;
        }

        _item = nullptr;
        Player* keyOwner = ObjectAccessor::FindPlayer(_map, m_ownerGuid);
        if (keyOwner)
            _item = keyOwner->GetItemByGuid(m_itemGuid);

        if (_item)
            keyOwner->ChallengeKeyCharded(_item, _challengeLevel);
        else
        {
            if (keyOwner = ObjectAccessor::FindPlayer(m_ownerGuid))
            {
                if (_item = keyOwner->GetItemByGuid(m_itemGuid))
                    keyOwner->ChallengeKeyCharded(_item, _challengeLevel);
                else
                    CharacterDatabase.PQuery("UPDATE challenge_key SET KeyIsCharded = 0, InstanceID = 0 WHERE guid = %u", m_ownerGuid.GetGUIDLow());
            }
            else
                CharacterDatabase.PQuery("UPDATE challenge_key SET KeyIsCharded = 0, InstanceID = 0 WHERE guid = %u", m_ownerGuid.GetGUIDLow());
        }
    }
}

bool Challenge::CanStart()
{
    if (_run)
        return true;

    return _map->GetPlayerCount() == _challengers.size(); // wait when group complete
}

void Challenge::Start()
{
    if (!_canRun)
        return;

    Player* keyOwner = ObjectAccessor::FindPlayer(m_ownerGuid);
    if (!keyOwner)
        return;

    _item = keyOwner->GetItemByGuid(m_itemGuid);
    if (!_item)
        return;

    keyOwner->m_challengeKeyInfo.InstanceID = keyOwner->GetInstanceId();
    keyOwner->m_challengeKeyInfo.needUpdate = true;

    _isKeyDepleted = false;

    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float o = 0.0f;

    if(!sChallengeMgr->GetStartPosition(_map->GetId(), x, y, z, o, m_ownerGuid))
        return;

    _map->ApplyOnEveryPlayer([&](Player* player)
    {
        if (player->CanContact())
        {
            player->SafeTeleport(_map->GetId(), x, y, z, o);
            _scenario->SendStepUpdate(player, true);
        }
    });

    BroadcastPacket(WorldPackets::ChallengeMode::ChallengeModeReset(_mapID).Write());

    SendStartTimer();
    SendChallengeModeStart();

    m_Functions.AddFunction([this]() -> void
    {
        if (!this)
            return;

        _challengeTimer = 0;
        SendStartElapsedTimer();

        for (ObjectGuid guid : GetInstanceScript()->_challengeDoorGuids)
            if (GameObject* challengeDoor = sObjectAccessor->FindGameObject(guid))
                challengeDoor->Delete();

        _run = true;

    }, m_Functions.CalculateTime(10 * IN_MILLISECONDS));

    _checkStart = false;

    if (HasAffix(Affixes::Quaking))
        _affixQuakingTimer = 20000;
}

void Challenge::Complete()
{
    if (_complete)
        return;

    _complete = true;

    Player* keyOwner = ObjectAccessor::FindPlayer(_map, m_ownerGuid);
    _item = nullptr;
    if (keyOwner)
        _item = keyOwner->GetItemByGuid(m_itemGuid);

    HitTimer();

    WorldPackets::Misc::StopElapsedTimer stopElapsedTimer;
    stopElapsedTimer.TimerID = WORLD_TIMER_TYPE_CHALLENGE_MODE;
    BroadcastPacket(stopElapsedTimer.Write());

    if (_challengeEntry)
    {
        WorldPackets::ChallengeMode::ChallengeModeComplete complete;
        complete.MapID = _mapID;
        complete.CompletionMilliseconds = _challengeTimer;
        complete.StartedChallengeLevel = _challengeLevel;
        complete.ChallengeID = _challengeEntry->ID;
        BroadcastPacket(complete.Write());
    }

    if (Group* group = sGroupMgr->GetGroupByGUID(m_gguid))
    {
        group->m_challengeEntry = nullptr;
        group->m_challengeLevel = 0;
        group->m_affixes.fill(0);
        group->SetDungeonDifficultyID(DIFFICULTY_MYTHIC_DUNGEON);
        group->m_challengeInstanceID = 0;
    }

    /// Reward part
    if (_item)
    {
        if (!_isKeyDepleted)
        {
            _item->SetModifier(ITEM_MODIFIER_CHALLENGE_ID, *Trinity::Containers::SelectRandomWeightedContainerElement(sDB2Manager.GetChallngeMaps(), sDB2Manager.GetChallngesWeight()));
            _item->SetModifier(ITEM_MODIFIER_CHALLENGE_KEYSTONE_LEVEL, std::min(_challengeLevel + _rewardLevel, sWorld->getIntConfig(CONFIG_CHALLENGE_LEVEL_LIMIT)));
        }
        else
            keyOwner->ChallengeKeyCharded(_item, _challengeLevel);

        keyOwner->UpdateChallengeKey(_item);
        _item->SetState(ITEM_CHANGED, keyOwner);
    }
    else
    {
        if (keyOwner = ObjectAccessor::FindPlayer(m_ownerGuid))
        {
            if (_item = keyOwner->GetItemByGuid(m_itemGuid))
                keyOwner->ChallengeKeyCharded(_item, _challengeLevel);
            else
                CharacterDatabase.PQuery("UPDATE challenge_key SET KeyIsCharded = 0, InstanceID = 0 WHERE guid = %u", m_ownerGuid.GetGUIDLow());
        }
        else
            CharacterDatabase.PQuery("UPDATE challenge_key SET KeyIsCharded = 0, InstanceID = 0 WHERE guid = %u", m_ownerGuid.GetGUIDLow());
    }

    auto challengeData = new ChallengeData;
    challengeData->ID = sObjectMgr->GetGenerator<HighGuid::Scenario>()->Generate();
    challengeData->MapID = _mapID;
    challengeData->RecordTime = _challengeTimer;
    challengeData->Date = time(nullptr);
    challengeData->ChallengeLevel = _challengeLevel;
    challengeData->TimerLevel = _rewardLevel;
    challengeData->ChallengeID = _challengeEntry ? _challengeEntry->ID : 0;
    challengeData->Affixes = _affixes;
    challengeData->GuildID = 0;
    if (InstanceScript* script = GetInstanceScript())
        challengeData->ChestID = script->_challengeChest.GetEntry();
    else
        challengeData->ChestID = 0;

    std::map<ObjectGuid::LowType /*guild*/, uint32> guildCounter;
    std::map<uint32, std::string> anticheatData;
    _map->ApplyOnEveryPlayer([&](Player* player)
    {
        ChallengeMember member;
        member.guid = player->GetGUID();
        member.specId = player->GetUInt32Value(PLAYER_FIELD_CURRENT_SPEC_ID);
        member.Date = time(nullptr);
        member.ChallengeLevel = _challengeLevel;
        if (InstanceScript* script = GetInstanceScript())
            member.ChestID = script->_challengeChest.GetEntry();
        else
            member.ChestID = 0;

        anticheatData[player->GetGUIDLow()] = player->GetName();

        if (player->GetGuildId())
            guildCounter[player->GetGuildId()]++;

        challengeData->member.insert(member);
        if (sChallengeMgr->CheckBestMemberMapId(member.guid, challengeData))
            SendChallengeModeNewPlayerRecord(player);

        SendChallengeModeMapStatsUpdate(player);

        player->UpdateAchievementCriteria(CRITERIA_TYPE_INSTANSE_MAP_ID, _mapID, _rewardLevel);

        player->RemoveAura(ChallengersBurden);
        player->CastSpell(player, SPELL_CHALLENGE_ANTIKICK, true);
        player->KilledMonsterCredit(542180); // for daily event quest
    });

    if (GetChallengeTimer() < 9 * MINUTE)
    {
        sLog->outWarden("Detect cheating challenge timer less 540 seconds, challengeMap %u", _mapID);
        std::string cheaters = "";
        for (auto const& record : anticheatData)
            cheaters += record.second + " (" + std::to_string(record.first) + ") ";
        sLog->outWarden("Challenge cheaters: %s", cheaters.c_str());
    }

    for (auto const& v : guildCounter)
        if (v.second >= 3)
            challengeData->GuildID = v.first;

    sChallengeMgr->SetChallengeMapData(challengeData->ID, challengeData);
    sChallengeMgr->CheckBestMapId(challengeData);
    sChallengeMgr->CheckBestGuildMapId(challengeData);
    sChallengeMgr->SaveChallengeToDB(challengeData);
}

void Challenge::HitTimer()
{
    if (GetChallengeTimer() < _chestTimers[2])
        _rewardLevel = CHALLENGE_TIMER_LEVEL_3; /// 3 chests + 3 levels
    else if (GetChallengeTimer() < _chestTimers[1])
        _rewardLevel = CHALLENGE_TIMER_LEVEL_2; /// 2 chests + 2 levels
    else if (GetChallengeTimer() < _chestTimers[0])
        _rewardLevel = CHALLENGE_TIMER_LEVEL_1; /// 1 chest + 1 level
    else
        _rewardLevel = CHALLENGE_NOT_IN_TIMER;

    if (!_map)
        return;

    for (auto const& guid : _challengers)
        _countItems[guid] = 0;

    switch (_rewardLevel)
    {
        case CHALLENGE_TIMER_LEVEL_3: /// 3 chests + 3 levels
        case CHALLENGE_TIMER_LEVEL_2: /// 2 chests + 2 levels
        case CHALLENGE_TIMER_LEVEL_1: /// 1 chest + 1 level
        case CHALLENGE_NOT_IN_TIMER:  /// 0 chest
        {
            if (InstanceScript* script = GetInstanceScript())
            {
                if (GameObject* chest = _map->GetGameObject(script->_challengeChest))
                    chest->SetRespawnTime(7 * DAY);

                float _chance = sChallengeMgr->GetChanceItem(_rewardLevel, _challengeLevel);
                auto countItems = int32(_chance / 100.0f);
                _chance -= countItems * 100.0f;

                if (roll_chance_f(_chance))
                    countItems++;

                while (countItems > 0)
                {
                    auto _tempList = _challengers;
                    Trinity::Containers::RandomResizeList(_tempList, countItems);

                    for (auto const& guid : _tempList)
                    {
                        countItems--;
                        _countItems[guid] += 1;
                    }
                }
            }
        }
        default:
            break;
    }
}

uint32 Challenge::GetChallengeLevel() const
{
    return std::min(_challengeLevel, sWorld->getIntConfig(CONFIG_CHALLENGE_LEVEL_LIMIT));
}

std::array<uint32, 3> Challenge::GetAffixes() const
{
    return _affixes;
}

bool Challenge::HasAffix(Affixes affix)
{
    return _affixesTest.test(size_t(affix));
}

uint32 Challenge::GetChallengeTimerToNow() const
{
    return (getMSTime() - _challengeTimer) / IN_MILLISECONDS;
}

void Challenge::BroadcastPacket(WorldPacket const* data) const
{
    _map->ApplyOnEveryPlayer([&](Player* player)
    {
        player->SendDirectMessage(data);
    });
}

uint32 Challenge::GetChallengeTimer()
{
    if (!_challengeTimer)
        return 0;

    return _challengeTimer / IN_MILLISECONDS;
}

void Challenge::ModChallengeTimer(uint32 timer)
{
    if (!timer)
        return;

    _challengeTimer += timer * IN_MILLISECONDS;
}

void Challenge::ResetGo()
{
    if (InstanceScript* script = GetInstanceScript())
        for (ObjectGuid guid : script->_challengeDoorGuids)
            if (GameObject* challengeDoor = sObjectAccessor->FindGameObject(guid))
                challengeDoor->SetGoState(GO_STATE_READY);

    if (InstanceScript* script = GetInstanceScript())
        if (GameObject* challengeOrb = _map->GetGameObject(script->_challengeOrbGuid))
        {
            challengeOrb->SetGoState(GO_STATE_READY);
            challengeOrb->RemoveFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_NODESPAWN);
        }
}

void Challenge::SendStartTimer(Player* player)
{
    WorldPackets::Instance::StartTimer startTimer;
    startTimer.Type = WORLD_TIMER_TYPE_CHALLENGE_MODE;
    startTimer.TimeRemaining = Seconds(ChallengeDelayTimer);
    startTimer.TotalTime = Seconds(ChallengeDelayTimer);
    if (player)
        player->SendDirectMessage(startTimer.Write());
    else
        BroadcastPacket(startTimer.Write());
}

void Challenge::SendStartElapsedTimer(Player* player)
{
    WorldPackets::WorldState::StartElapsedTimer timer;
    timer.Timer.TimerID = WORLD_TIMER_TYPE_CHALLENGE_MODE;
    timer.Timer.CurrentDuration = GetChallengeTimer();

    if (player)
        player->SendDirectMessage(timer.Write());
    else
        BroadcastPacket(timer.Write());
}

void Challenge::SendChallengeModeStart(Player* player)
{
    if (!_challengeEntry)
        return;

    WorldPackets::ChallengeMode::ChallengeModeStart start;
    start.MapID = _mapID;
    start.ChallengeID = _challengeEntry->ID;
    start.StartedChallengeLevel = _challengeLevel;
    start.Affixes = _affixes;

    if (player)
        player->SendDirectMessage(start.Write());
    else
        BroadcastPacket(start.Write());
}

void Challenge::SendChallengeModeNewPlayerRecord(Player* player)
{
    WorldPackets::ChallengeMode::ChallengeModeNewPlayerRecord newRecord;
    newRecord.MapID = _mapID;
    newRecord.CompletionMilliseconds = _challengeTimer;
    newRecord.StartedChallengeLevel = _rewardLevel;

    if (player)
        player->SendDirectMessage(newRecord.Write());
}

void Challenge::SendChallengeModeMapStatsUpdate(Player* player)
{
    if (!_challengeEntry)
        return;

    ChallengeByMap* bestMap = sChallengeMgr->BestForMember(player->GetGUID());
    if (!bestMap)
        return;

    auto itr = bestMap->find(_mapID);
    if (itr == bestMap->end())
        return;

    ChallengeData* best = itr->second;
    if (!best)
        return;

    WorldPackets::ChallengeMode::ChallengeModeMapStatsUpdate update;
    update.Stats.MapId = _mapID;
    update.Stats.BestCompletionMilliseconds = best->RecordTime;
    update.Stats.LastCompletionMilliseconds = _challengeTimer;
    update.Stats.ChallengeID = _challengeEntry->ID;
    update.Stats.BestMedalDate = best->Date;
    update.Stats.Affixes = best->Affixes;

    ChallengeMemberList members = best->member;
    for (auto const& v : members)
        update.Stats.BestSpecID.push_back(v.specId);

    if (player)
        player->SendDirectMessage(update.Write());
}

void Challenge::SummonWall(Player* /*player*/)
{
    if (InstanceScript* script = GetInstanceScript())
        for (ObjectGuid guid : script->_challengeDoorGuids)
            if (GameObject* challengeDoor = sObjectAccessor->FindGameObject(guid))
                challengeDoor->SetRespawnTime(7 * DAY);
}

void Challenge::SetInstanceScript(InstanceScript* instanceScript)
{
    _instanceScript = instanceScript;
}

InstanceScript* Challenge::GetInstanceScript() const
{
    return _instanceScript;
}

uint8 Challenge::GetItemCount(ObjectGuid guid) const
{
    auto itr = _countItems.find(guid);
    if (itr == _countItems.end())
        return 0;

    return itr->second;
}

uint8 Challenge::GetLevelBonus() const
{
    switch (_rewardLevel)
    {
        case CHALLENGE_TIMER_LEVEL_3: /// 3 chests + 3 levels
            return 2;
        case CHALLENGE_TIMER_LEVEL_2: /// 2 chests + 2 levels
            return 1;
        case CHALLENGE_TIMER_LEVEL_1: /// 1 chest + 1 level
        case CHALLENGE_NOT_IN_TIMER:  /// 0 chest
        default:
            return 0;
    }
}
