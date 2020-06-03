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

#include "ChallengeModePackets.h"
#include "ChallengeMgr.h"
#include "Challenge.h"
#include "Chat.h"
#include "Group.h"
#include "InstancePackets.h"
#include "ScenarioMgr.h"

void WorldSession::HandleRequestLeaders(WorldPackets::ChallengeMode::RequestLeaders& packet)
{
    WorldPackets::ChallengeMode::RequestLeadersResult result;
    result.MapID = packet.MapId;
    result.ChallengeID = packet.ChallengeID;

    result.LastGuildUpdate = time(nullptr);
    result.LastRealmUpdate = time(nullptr);

    if (auto bestGuild = sChallengeMgr->BestGuildChallenge(_player->GetGuildId(), packet.ChallengeID))
    {
        for (auto itr = bestGuild->member.begin(); itr != bestGuild->member.end(); ++itr)
        {
            WorldPackets::ChallengeMode::ModeAttempt guildLeaders;
            guildLeaders.InstanceRealmAddress = GetVirtualRealmAddress();
            guildLeaders.AttemptID = bestGuild->ID;
            guildLeaders.CompletionTime = bestGuild->RecordTime;
            guildLeaders.CompletionDate = bestGuild->Date;
            guildLeaders.MedalEarned = bestGuild->ChallengeLevel;

            for (auto const& v : bestGuild->member)
            {
                WorldPackets::ChallengeMode::ModeAttempt::Member memberData;
                memberData.VirtualRealmAddress = GetVirtualRealmAddress();
                memberData.NativeRealmAddress = GetVirtualRealmAddress();
                memberData.Guid = v.guid;
                memberData.SpecializationID = v.specId;
                guildLeaders.Members.emplace_back(memberData);
            }

            result.GuildLeaders.emplace_back(guildLeaders);
        }
    }

    if (ChallengeData* bestServer = sChallengeMgr->BestServerChallenge(packet.ChallengeID))
    {
        WorldPackets::ChallengeMode::ModeAttempt realmLeaders;
        realmLeaders.InstanceRealmAddress = GetVirtualRealmAddress();
        realmLeaders.AttemptID = bestServer->ID;
        realmLeaders.CompletionTime = bestServer->RecordTime;
        realmLeaders.CompletionDate = bestServer->Date;
        realmLeaders.MedalEarned = bestServer->ChallengeLevel;

        for (auto const& v : bestServer->member)
        {
            WorldPackets::ChallengeMode::ModeAttempt::Member memberData;
            memberData.VirtualRealmAddress = GetVirtualRealmAddress();
            memberData.NativeRealmAddress = GetVirtualRealmAddress();
            memberData.Guid = v.guid;
            memberData.SpecializationID = v.specId;
            realmLeaders.Members.emplace_back(memberData);
        }
        result.RealmLeaders.push_back(realmLeaders);
    }

    SendPacket(result.Write());
}

void WorldSession::HandleGetChallengeModeRewards(WorldPackets::ChallengeMode::Misc& /*packet*/)
{
    //WorldPackets::ChallengeMode::Rewards rewards;
    //SendPacket(rewards.Write());
}

void WorldSession::HandleChallengeModeRequestMapStats(WorldPackets::ChallengeMode::Misc& /*packet*/)
{
    WorldPackets::ChallengeMode::AllMapStats stats;
    if (ChallengeByMap* last = sChallengeMgr->LastForMember(_player->GetGUID()))
    {
        for (auto const& v : *last)
        {
            WorldPackets::ChallengeMode::ChallengeModeMap modeMap;
            modeMap.ChallengeID = v.second->ChallengeID;
            modeMap.BestMedalDate = v.second->Date;
            modeMap.MapId = v.second->MapID;
            modeMap.CompletedChallengeLevel = v.second->ChallengeLevel;

            modeMap.LastCompletionMilliseconds = v.second->RecordTime;
            if (ChallengeData* _lastData = sChallengeMgr->BestForMemberMap(_player->GetGUID(), v.second->ChallengeID))
                modeMap.BestCompletionMilliseconds = _lastData->RecordTime;
            else
                modeMap.BestCompletionMilliseconds = v.second->RecordTime;

            modeMap.Affixes = v.second->Affixes;

            for (auto const& z : v.second->member)
                modeMap.BestSpecID.push_back(z.specId);

            stats.ChallengeModeMaps.push_back(modeMap);
        }
    }

    SendPacket(stats.Write());
}

void WorldSession::HandleStartChallengeMode(WorldPackets::ChallengeMode::StartChallengeMode& packet)
{
    if (packet.GameObjectGUID.GetEntry() != ChallengeModeOrb || !sWorld->getBoolConfig(CONFIG_CHALLENGE_ENABLED))
        return;

    if (Item* item = _player->GetItemByEntry(138019))
    {
        InstanceMap* inst = _player->GetMap()->ToInstanceMap();
        if (!inst)
        {
            ChatHandler(_player).PSendSysMessage("Error: Is not a Instance Map.");
            return;
        }

        if (inst->GetSpawnMode() == DIFFICULTY_MYTHIC_KEYSTONE)
        {
            ChatHandler(_player).PSendSysMessage("Error: For run Mythic please rerun instance.");
            return;
        }

        if (_player->m_challengeKeyInfo.InstanceID)
        {
            ChatHandler(_player).PSendSysMessage("Error: Key allready run in other instance.");
            return;
        }

        if (_player->m_challengeKeyInfo.Level < 2)
        {
            _player->ChallengeKeyCharded(nullptr, _player->m_challengeKeyInfo.Level, false); // Deleted bugged key
            ChatHandler(_player).PSendSysMessage("Error: Key is bugged.");
            return;
        }

        float x = 0.0f; float y = 0.0f; float z = 0.0f; float o = 0.0f;
        if(!sChallengeMgr->GetStartPosition(_player->GetMapId(), x, y, z, o, _player->GetGUID()))
        {
            ChatHandler(_player).PSendSysMessage("Error: Start position not found.");
            return;
        }

        if (!_player->isAlive())
        {
            ChatHandler(_player).PSendSysMessage("Error: Player not found or die.");
            return;
        }

        if (_player->isInCombat())
        {
            ChatHandler(_player).PSendSysMessage("Error: Player in combat.");
            return;
        }

        if (Group* group = _player->GetGroup())
        {
            for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
            {
                Player* player = itr->getSource();
                if (!player || !player->isAlive())
                {
                    ChatHandler(_player).PSendSysMessage("Error: Player not found or die.");
                    return;
                }

                if (player->isInCombat())
                {
                    ChatHandler(_player).PSendSysMessage("Error: Player in combat.");
                    return;
                }

                if (!player->GetMap() || player->GetMap()->ToInstanceMap() != inst)
                {
                    ChatHandler(_player).PSendSysMessage("Error: Player in group not this map.");
                    return;
                }
            }

            group->m_challengeEntry = sMapChallengeModeStore.LookupEntry(_player->m_challengeKeyInfo.ID);
            group->m_affixes.fill(0);

            MapChallengeModeEntry const* challengeEntry = sDB2Manager.GetChallengeModeByMapID(_player->GetMapId());
            if (!group->m_challengeEntry || !challengeEntry || challengeEntry->MapID != group->m_challengeEntry->MapID)
            {
                group->m_challengeEntry = nullptr;
                ChatHandler(_player).PSendSysMessage("Error: Is not this challenge.");
                return;
            }

            group->m_challengeOwner = _player->GetGUID();
            group->m_challengeItem = item->GetGUID();
            group->m_challengeLevel = _player->m_challengeKeyInfo.Level;

            if (group->m_challengeLevel > 3)
                group->m_affixes[0] = _player->m_challengeKeyInfo.Affix;
            if (group->m_challengeLevel > 6)
                group->m_affixes[1] = _player->m_challengeKeyInfo.Affix1;
            if (group->m_challengeLevel > 9)
                group->m_affixes[2] = _player->m_challengeKeyInfo.Affix2;

            WorldPackets::Instance::ChangePlayerDifficultyResult result;
            result.Result = AsUnderlyingType(ChangeDifficultyResult::DIFFICULTY_CHANGE_SET_COOLDOWN_S);
            result.CooldownReason = 2813862382;
            group->BroadcastPacket(result.Write(), true);

            WorldPackets::Instance::ChangePlayerDifficultyResult result2;
            result2.Result = AsUnderlyingType(ChangeDifficultyResult::DIFFICULTY_CHANGE_BY_PARTY_LEADER);
            result2.InstanceMapID = _player->GetMapId();
            result2.DifficultyRecID = DIFFICULTY_MYTHIC_KEYSTONE;

            group->SetDungeonDifficultyID(DIFFICULTY_MYTHIC_KEYSTONE);

            for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
            {
                if (Player* player = itr->getSource())
                {
                    player->SetDungeonDifficultyID(DIFFICULTY_MYTHIC_KEYSTONE);
                    player->SendDirectMessage(result2.Write());
                    player->TeleportToChallenge(_player->GetMapId(), x, y, z, o);
                    player->CastSpell(player, ChallengersBurden, true);
                }
            }
            if (GameObject* challengeOrb = sObjectAccessor->FindGameObject(packet.GameObjectGUID))
            {
                challengeOrb->SetGoState(GO_STATE_ACTIVE);
                challengeOrb->SetFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_NODESPAWN);
            }
        }
        else
        {
            _player->m_challengeKeyInfo.challengeEntry = sMapChallengeModeStore.LookupEntry(_player->m_challengeKeyInfo.ID);

            MapChallengeModeEntry const* challengeEntry = sDB2Manager.GetChallengeModeByMapID(_player->GetMapId());
            if (!_player->m_challengeKeyInfo.challengeEntry || !challengeEntry || challengeEntry->MapID != _player->m_challengeKeyInfo.challengeEntry->MapID)
            {
                _player->m_challengeKeyInfo.challengeEntry = nullptr;
                ChatHandler(_player).PSendSysMessage("Error: Is not this challenge.");
                return;
            }

            WorldPackets::Instance::ChangePlayerDifficultyResult result;
            result.Result = AsUnderlyingType(ChangeDifficultyResult::DIFFICULTY_CHANGE_SET_COOLDOWN_S);
            result.CooldownReason = 2813862382;
            SendPacket(result.Write(), true);

            WorldPackets::Instance::ChangePlayerDifficultyResult result2;
            result2.Result = AsUnderlyingType(ChangeDifficultyResult::DIFFICULTY_CHANGE_BY_PARTY_LEADER);
            result2.InstanceMapID = _player->GetMapId();
            result2.DifficultyRecID = DIFFICULTY_MYTHIC_KEYSTONE;

            _player->SetDungeonDifficultyID(DIFFICULTY_MYTHIC_KEYSTONE);
            SendPacket(result2.Write());
            _player->TeleportToChallenge(_player->GetMapId(), x, y, z, o);
            _player->CastSpell(_player, ChallengeSpells::ChallengersBurden, true);

            if (GameObject* challengeOrb = sObjectAccessor->FindGameObject(packet.GameObjectGUID))
            {
                challengeOrb->SetGoState(GOState::GO_STATE_ACTIVE);
                challengeOrb->SetFlag(GameObjectFields::GAMEOBJECT_FIELD_FLAGS, GameObjectFlags::GO_FLAG_NODESPAWN);
            }
        }
    }
}

void WorldSession::HandleResetChallengeMode(WorldPackets::ChallengeMode::ResetChallengeMode& /*packet*/)
{
    if (auto const& instanceScript = _player->GetInstanceScript())
        if (instanceScript->instance->isChallenge())
            instanceScript->ResetChallengeMode();
}
