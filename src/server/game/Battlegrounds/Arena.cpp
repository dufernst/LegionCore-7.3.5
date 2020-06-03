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

#include "Arena.h"
#include "Bracket.h"
#include "Group.h"
#include "GuildMgr.h"
#include "ArenaScore.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "Packets/WorldStatePackets.h"
#include "BattlegroundMgr.h"
#include "Battleground.h"

Arena::Arena()
{
    for (uint8 i = BG_STARTING_EVENT_FIRST; i < BG_STARTING_EVENT_COUNT; ++i)
        m_broadcastMessages[i] = ArenaBroadcastTexts[i];

    _dampeningTimer.SetInterval(Seconds(3).count());
    _winConditionCheckTimer.SetInterval(Seconds(3).count());
    _logData = {};
}

void Arena::PostUpdateImpl(uint32 diff)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    _dampeningTimer.Update(diff);
    _winConditionCheckTimer.Update(diff);

    if (_dampeningTimer.OnTimerPassReset())
        ApplyDampeningIfNeeded();

    Milliseconds elapsedTime = GetElapsedTime();
    if (elapsedTime >= Minutes(25))
    {
        UpdateArenaWorldState();
        CheckWinConditions();
        return;
    }

    if (_winConditionCheckTimer.OnTimerPassReset())
        CheckWinConditions();

    if (elapsedTime > Minutes(2))
        UpdateWorldState(ARENA_END_TIMER, int32(time(nullptr) + std::chrono::duration_cast<Seconds>(Minutes(25) - elapsedTime).count()));

    ModifyStartDelayTime(Milliseconds(diff));
}

void Arena::_ProcessJoin(uint32 diff)
{
    Battleground::_ProcessJoin(diff);

    if (GetStartDelayTime() <= m_messageTimer[BG_STARTING_EVENT_FOURTH] && !(m_Events & BG_STARTING_EVENT_4))
    {
        uint8 bgQueueTypeId = MS::Battlegrounds::GetBgQueueTypeIdByBgTypeID(GetTypeID(), GetJoinType());

        for (auto const& itr : GetPlayers())
            if (Player* player = ObjectAccessor::FindPlayer(itr.first))
            {
                WorldPackets::Battleground::BattlefieldStatusActive battlefieldStatus;
                sBattlegroundMgr->BuildBattlegroundStatusActive(&battlefieldStatus, this, player, player->GetBattlegroundQueueIndex(bgQueueTypeId), player->GetBattlegroundQueueJoinTime(bgQueueTypeId), GetJoinType());
                player->SendDirectMessage(battlefieldStatus.Write());

                player->RemoveAurasDueToSpell(SPELL_ARENA_PREPARATION);
                player->ResetAllPowers();

                player->RemoveAppliedAuras([](AuraApplicationPtr const aurApp)
                {
                    Aura* aura = aurApp->GetBase();
                    return !aura->IsPermanent() && aura->GetDuration() <= 30 * IN_MILLISECONDS && aurApp->IsPositive() && (!(aura->GetSpellInfo()->HasAttribute(SPELL_ATTR0_UNAFFECTED_BY_INVULNERABILITY))) && (!aura->HasEffectType(SPELL_AURA_MOD_INVISIBILITY));
                });
            }

        CheckWinConditions();
    }
}

void Arena::AddPlayer(Player* player)
{
    Battleground::AddPlayer(player);

    PlayerScores[player->GetGUID()] = new ArenaScore(player->GetGUID(), player->GetBGTeamId());

    player->ResummonPetTemporaryUnSummonedIfAny();

    if (Pet* pet = player->GetPet())
    {
        if (!pet->isAlive())
            pet->setDeathState(ALIVE);

        pet->SetHealth(pet->GetMaxHealth());
        pet->RemoveAllAuras();

        if (player->HasSpell(155228) || player->HasSpell(205024) || player->GetSpecializationId() == SPEC_MAGE_FIRE &&
            player->GetSpecializationId() == SPEC_MAGE_ARCANE || player->GetSpecializationId() == SPEC_DK_BLOOD || player->GetSpecializationId() == SPEC_DK_FROST)
            player->RemovePet(pet);
    }

    player->RemoveArenaEnchantments(TEMP_ENCHANTMENT_SLOT);

    uint32 team = player->GetTeam();
    if (team == ALLIANCE)
        player->CastSpell(player, player->GetBGTeam() == HORDE ? SPELL_BG_ALLIANCE_GREEN_FLAG : SPELL_BG_ALLIANCE_GOLD_FLAG, true);
    else
        player->CastSpell(player, player->GetBGTeam() == HORDE ? SPELL_BG_HORDE_GREEN_FLAG : SPELL_BG_HORDE_GOLD_FLAG, true);

    player->DestroyConjuredItems(true);
    player->UnsummonPetTemporaryIfAny();

    if (GetStatus() == STATUS_WAIT_JOIN)
    {
        player->CastSpell(player, SPELL_ARENA_PREPARATION, true);
        player->CastSpell(player, SPELL_ARENA_PERIODIC_AURA, true);
        player->CastSpell(player, SPELL_ENTERING_BATTLEGROUND, true);
        if (IsRated())
            player->CastSpell(player, SPELL_RATED_PVP_TRANSFORM_SUPPRESSION, true);

        player->ResetAllPowers();
    }

    if (!player->IsSpectator())
    {
        SendOpponentSpecialization(team);
        SendOpponentSpecialization(GetOtherTeam(team));
    }

    UpdateArenaWorldState();
}

void Arena::RemovePlayer(Player* /*player*/, ObjectGuid /*guid*/, uint32 /*team*/)
{
    if (GetStatus() == STATUS_WAIT_LEAVE)
        return;

    UpdateArenaWorldState();
    CheckWinConditions();
}

void Arena::FillInitialWorldStates(WorldPackets::WorldState::InitWorldStates& packet)
{
    packet.Worldstates.emplace_back(ARENA_SHOW_END_TIMER, GetStatus() == STATUS_IN_PROGRESS);
    packet.Worldstates.emplace_back(ARENA_END_TIMER, int32(time(nullptr) + std::chrono::duration_cast<Seconds>(Minutes(25) - GetElapsedTime()).count()));
    packet.Worldstates.emplace_back(ARENA_ALIVE_PLAYERS_GREEN, GetAlivePlayersCountByTeam(HORDE));
    packet.Worldstates.emplace_back(ARENA_ALIVE_PLAYERS_GOLD, GetAlivePlayersCountByTeam(ALLIANCE));
    packet.Worldstates.emplace_back(BG_RV_WORLD_STATE, 1);
}

void Arena::UpdateArenaWorldState()
{
    UpdateWorldState(ARENA_ALIVE_PLAYERS_GREEN, GetAlivePlayersCountByTeam(HORDE));
    UpdateWorldState(ARENA_ALIVE_PLAYERS_GOLD, GetAlivePlayersCountByTeam(ALLIANCE));
}

void Arena::HandleKillPlayer(Player* player, Player* killer)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    Battleground::HandleKillPlayer(player, killer);

    UpdateArenaWorldState();
    CheckWinConditions();
}

void Arena::StartingEventCloseDoors()
{
    UpdateWorldState(ARENA_SHOW_END_TIMER, 0);
    Battleground::StartingEventCloseDoors();
}

void Arena::StartingEventOpenDoors()
{
    UpdateWorldState(ARENA_SHOW_END_TIMER, 1);

    CheckWinConditions();

    for (auto const& v : GetPlayers())
        if (Player* player = ObjectAccessor::FindPlayer(v.first))
        {
            player->RemoveAurasDueToSpell(SPELL_ARENA_PREPARATION);
            player->ResetAllPowers();

            player->RemoveAppliedAuras([](AuraApplicationPtr const aurApp)
            {
                Aura* aura = aurApp->GetBase();
                return !aura->IsPermanent() && aura->GetDuration() <= 30 * IN_MILLISECONDS && aurApp->IsPositive() && (!(aura->GetSpellInfo()->HasAttribute(SPELL_ATTR0_UNAFFECTED_BY_INVULNERABILITY))) && (!aura->HasEffectType(SPELL_AURA_MOD_INVISIBILITY));
            });
        }

    Battleground::StartingEventOpenDoors();

    _logData = {};
    _logData.RealmID = realm.Id.Realm;
    _logData.MapID = GetMapId();
    _logData.Arena = boost::in_place();
    _logData.Arena->JoinType = GetJoinType();

    for (const auto& itr : GetPlayers())
    {
        if (auto player = ObjectAccessor::FindPlayer(itr.first))
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
}

void Arena::RemovePlayerAtLeave(ObjectGuid guid, bool transport, bool sendPacket)
{
    if (Player* player = ObjectAccessor::FindPlayer(guid))
    {
        player->RemoveAurasDueToSpell(SPELL_ARENA_PERIODIC_AURA);
        player->RemoveAurasDueToSpell(SPELL_ENTERING_BATTLEGROUND);
        if (IsRated())
            player->RemoveAurasDueToSpell(SPELL_RATED_PVP_TRANSFORM_SUPPRESSION);
    }

    Battleground::RemovePlayerAtLeave(guid, transport, sendPacket);
}

void Arena::CheckWinConditions()
{
    if (!GetAlivePlayersCountByTeam(ALLIANCE) && GetPlayersCountByTeam(HORDE))
        EndBattleground(HORDE);
    else if (GetPlayersCountByTeam(ALLIANCE) && !GetAlivePlayersCountByTeam(HORDE))
        EndBattleground(ALLIANCE);

    if (GetElapsedTime() >= Minutes(25))
    {
        if (GetAlivePlayersCountByTeam(ALLIANCE) < GetPlayersCountByTeam(HORDE))
            EndBattleground(HORDE);
        else if (GetPlayersCountByTeam(ALLIANCE) > GetAlivePlayersCountByTeam(HORDE))
            EndBattleground(ALLIANCE);
        else
            EndBattleground(WINNER_NONE);
    }
}

void Arena::ApplyDampeningIfNeeded()
{
    auto applyDampening([=]() -> void
    {
        for (auto const& itr : GetPlayers())
            if (auto const& player = GetPlayer(itr, "ApplyDampeningIfNeeded"))
                if (!player->HasAura(SPELL_BG_ARENA_DUMPENING))
                    player->AddAura(SPELL_BG_ARENA_DUMPENING, player);
    });

    uint8 joinType = GetJoinType();
    if (GetBrawlJoinType())
        joinType = GetBrawlJoinType();

    switch (joinType)
    {
        case MS::Battlegrounds::JoinType::Arena2v2:
            applyDampening();
            break;
        case MS::Battlegrounds::JoinType::ArenaSoloQ3v3:
        case MS::Battlegrounds::JoinType::Arena3v3:
            if (GetElapsedTime() >= std::chrono::minutes(5))
                applyDampening();
            break;
        default:
            break;
    }
}

void Arena::EndBattleground(uint32 winner)
{
    if (IsRated())
    {
        _logData.Arena->WinnerTeamId = winner;
        _logData.Arena->Duration = GetElapsedTime().count();
        _logData.Arena->WinnerOldRating = GetMatchmakerRating(winner);
        _logData.Arena->WinnerNewRating = 0;
        _logData.Arena->LooserOldRating = GetMatchmakerRating(GetOtherTeam(winner));
        _logData.Arena->LooserNewRating = 0; // _arenaTeamScores[MS::Battlegrounds::GetTeamIdByTeam(winner)].NewRating;

        for (const auto& itr : GetPlayers())
        {
            if (auto player = ObjectAccessor::FindPlayer(itr.first))
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
            }
        }

        sLog->OutPveEncounter(_logData.Serealize().c_str());
        _logData = {};
    }

    if (IsRated() && winner != WINNER_NONE && GetStatus() == STATUS_IN_PROGRESS && GetJoinType() != MS::Battlegrounds::JoinType::Arena1v1)
    {
        std::string winnerTeam;
        std::string loserTeam;
        bool attention = HandleArenaLogPlayerNames(winnerTeam, loserTeam, "!?$", winner);
        uint32 _arenaTimer = GetElapsedTime().count();
        uint32 _min = _arenaTimer / IN_MILLISECONDS / 60;
        uint32 _sec = (_arenaTimer - _min * 60 * IN_MILLISECONDS) / IN_MILLISECONDS;
        std::string att;

        switch (GetJoinType())
        {
            case MS::Battlegrounds::JoinType::Arena1v1:
            {
                if (attention || GetElapsedTime() < Seconds(50))
                    att += "--- ATTENTION!";

                // sLog->outArena("FINISH: Arena match Type: 1v1 --- Winner[%s]: old rating: %u --- Loser[%s]: old rating: %u --- Duration: %u min. %u sec. %s",
                //              winnerTeam.c_str(), GetMatchmakerRating(winner), loserTeam.c_str(), GetMatchmakerRating(GetOtherTeam(winner)), _min, _sec, att.c_str());
                break;
            }
            case MS::Battlegrounds::JoinType::Arena2v2:
            {
                if (attention || GetElapsedTime() < Seconds(70))
                    att += "--- ATTENTION!";

                sLog->outArena(MS::Battlegrounds::JoinType::Arena2v2, "FINISH: Arena match Type: 2v2 --- Winner[%s]: old rating: %u --- Loser[%s]: old rating: %u --- Duration: %u min. %u sec. %s",
                               winnerTeam.c_str(), GetMatchmakerRating(winner), loserTeam.c_str(), GetMatchmakerRating(GetOtherTeam(winner)), _min, _sec, att.c_str());
                break;
            }
            case MS::Battlegrounds::JoinType::Arena3v3:
            {
                if (attention || GetElapsedTime() < Seconds(100))
                    att += "--- ATTENTION!";

                sLog->outArena(MS::Battlegrounds::JoinType::Arena3v3, "FINISH: Arena match Type: 3v3 --- Winner[%s]: old rating: %u --- Loser[%s]: old rating: %u --- Duration: %u min. %u sec. %s",
                               winnerTeam.c_str(), GetMatchmakerRating(winner), loserTeam.c_str(), GetMatchmakerRating(GetOtherTeam(winner)), _min, _sec, att.c_str());
                break;
            }
            default:
                sLog->outArena(GetJoinType(), "match Type: %u --- Winner: old rating: %u  --- Loser: old rating: %u| DETAIL: Winner[%s] Loser[%s]",
                    GetJoinType(), GetMatchmakerRating(winner), GetMatchmakerRating(GetOtherTeam(winner)), winnerTeam.c_str(), loserTeam.c_str());
                break;
        }
    }

    Battleground::EndBattleground(winner);
}

void Arena::SendOpponentSpecialization(uint32 team)
{
    WorldPackets::Battleground::ArenaPrepOpponentSpecializations spec;

    for (auto const& itr : GetPlayers())
        if (auto const& opponent = GetPlayer(itr, "SendOponentSpecialization"))
        {
            if (itr.second.Team != team)
                continue;

            WorldPackets::Battleground::ArenaPrepOpponentSpecializations::OpponentSpecData data;
            data.Guid = opponent->GetGUID();
            data.SpecializationID = opponent->GetUInt32Value(PLAYER_FIELD_CURRENT_SPEC_ID);
            spec.Data.emplace_back(data);
        }

    SendPacketToTeam(GetOtherTeam(team), spec.Write());
}
