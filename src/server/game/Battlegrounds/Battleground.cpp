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

#include "Arena.h"
#include "Battleground.h"
#include "BattlegroundMgr.h"
#include "BattlegroundPackets.h"
#include "BattlegroundDM.h"
#include "Bracket.h"
#include "BracketMgr.h"
#include "ChatPackets.h"
#include "ChatTextBuilder.h"
#include "Creature.h"
#include "CreatureTextMgr.h"
#include "Duration.h"
#include "Formulas.h"
#include "GameObjectPackets.h"
#include "GridNotifiersImpl.h"
#include "Group.h"
#include "GroupMgr.h"
#include "Guild.h"
#include "GuildMgr.h"
#include "InstancePackets.h"
#include "Language.h"
#include "MapManager.h"
#include "MiscPackets.h"
#include "Object.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "PlayerDefines.h"
#include "QuestData.h"
#include "SpectatorAddon.h"
#include "Util.h"
#include "World.h"
#include "WorldPacket.h"
#include "WorldStatePackets.h"
#include <G3D/Quat.h>

template<class Do>
void Battleground::BroadcastWorker(Do& _do)
{
    for (auto const& itr : GetPlayers())
        if (Player* player = ObjectAccessor::FindPlayer(itr.first))
            _do(player);
}

Battleground::Battleground() : m_PrematureCountDownTimer(0), m_ArenaTeamMMR{}, ScriptId(0)
{
    m_QueueID = 0;
    m_TypeID = MS::Battlegrounds::BattlegroundTypeId::None;
    m_RandomTypeID = MS::Battlegrounds::BattlegroundTypeId::None;
    m_InstanceID = 0;
    m_Status = STATUS_NONE;
    m_ClientInstanceID = 0;
    m_EndTime = Milliseconds(0);
    m_LastResurrectTime = 0;
    m_BracketId = 0;
    m_InvitedAlliance = 0;
    m_InvitedHorde = 0;
    m_JoinType = MS::Battlegrounds::JoinType::None;
    m_IsArena = false;
    m_Winner = 2;

    m_StartTime = Milliseconds(0);
    m_CountdownTimer = Milliseconds(0);
    m_ResetStatTimer = 0;
    m_ValidStartPositionTimer = 0;

    m_Events = 0;
    m_IsRated = false;
    _useTournamentRules = false;
    _isSkirmish = false;
    _isWargame = false;

    m_BuffChange = false;
    m_Name = "";
    m_LevelMin = 0;
    m_LevelMax = 0;
    m_InBGFreeSlotQueue = false;
    m_SetDeleteThis = false;

    m_MaxPlayersPerTeam = 0;
    m_MaxPlayers = 0;
    m_MinPlayersPerTeam = 0;
    m_MinPlayers = 0;

    m_MapId = 0;
    m_Map = nullptr;

    m_PrematureCountDown = false;

    for (uint8 i = BG_STARTING_EVENT_FIRST; i < BG_STARTING_EVENT_COUNT; ++i)
        m_broadcastMessages[i] = BattlegroundBroadcastTexts[i];

    m_IsRBG = false;
    _isBrawl = false;
    m_IsBG = false;

    m_LastPlayerPositionBroadcast = PositionBroadcastUpdate;

    m_StartDelayTime = Milliseconds(0);

    for (int8 i = TEAM_ALLIANCE; i < MAX_TEAMS; ++i)
    {
        m_TeamStartPos[i] = {};
        m_GroupIds[i] = 0;
        m_BgRaids[i] = nullptr;
        m_PlayersCount[i] = 0;
        m_TeamScores[i] = 0;
    }

    m_allMembers.clear();
    m_lastFlagCaptureTeam = 0;
}

Battleground::~Battleground()
{
    m_OfflineQueue.clear();
    auto size = uint32(BgCreatures.size());
    for (uint32 i = 0; i < size; ++i)
        DelCreature(i);

    size = uint32(BgObjects.size());
    for (uint32 i = 0; i < size; ++i)
        DelObject(i);

    sBattlegroundMgr->RemoveBattleground(GetInstanceID(), GetTypeID());

    if (m_Map)
    {
        m_Map->SetUnload();
        m_Map->SetBG(nullptr);
        m_Map = nullptr;
    }

    RemoveFromBGFreeSlotQueue();

    for (std::map<ObjectGuid, BattlegroundScore*>::const_iterator itr = PlayerScores.begin(); itr != PlayerScores.end(); ++itr)
        delete itr->second;
}

void Battleground::Update(uint32 diff)
{
    if (_players.empty())
    {
        if (!GetInvitedCount(HORDE) && !GetInvitedCount(ALLIANCE))
            m_SetDeleteThis = true;
        return;
    }

    _CheckPositions(diff);

    switch (GetStatus())
    {
        case STATUS_WAIT_JOIN:
            if (!_players.empty())
                _ProcessJoin(diff);
            break;
        case STATUS_IN_PROGRESS:
            _ProcessOfflineQueue();
            _ProcessPlayerPositionBroadcast(Milliseconds(diff));
            if (!IsArena() || GetTypeID() == MS::Battlegrounds::BattlegroundTypeId::BrawlBattlegroundShadoPan)
            {
                //_ProcessPlayerPositionBroadcast(Milliseconds(diff));
                _ProcessRessurect(diff);
                if (sBattlegroundMgr->GetPrematureFinishTime() && 
                ((GetPlayersCountByTeam(ALLIANCE) < GetMinPlayersPerTeam() || GetPlayersCountByTeam(HORDE) < GetMinPlayersPerTeam()) && GetMapId() != 1101 || // if one team has smaller players, that need and not DM
                 GetMapId() == 1101 && GetBattlegroundScoreMap().size() < GetMinPlayersPerTeam()))   // or DM and summary players smaller that summary need
                    _ProcessProgress(diff);
                else if (m_PrematureCountDown)
                    m_PrematureCountDown = false;
            }
            break;
        case STATUS_WAIT_LEAVE:
            _ProcessLeave(diff);
            break;
        default:
            break;
    }

    m_StartTime = GetElapsedTime() + Milliseconds(diff);
    if (GetStatus() == STATUS_WAIT_JOIN)
    {
        m_ResetStatTimer += diff;
        m_CountdownTimer += Milliseconds(diff);
    }

    PostUpdateImpl(diff);
    PostUpdateImpl(Milliseconds(diff));
}

void Battleground::Initialize(CreateBattlegroundData const* data)
{
    m_MapId = data->MapID;
    m_TypeID = data->bgTypeId;
    m_IsArena = data->IsArena;
    m_IsRBG = data->IsRbg;
    m_MinPlayersPerTeam = data->MinPlayersPerTeam;
    m_MaxPlayersPerTeam = data->MaxPlayersPerTeam;
    m_MinPlayers = m_MinPlayersPerTeam * 2;
    m_MaxPlayers = m_MaxPlayersPerTeam * 2;
    m_Name = data->BattlegroundName;
    m_LevelMin = data->LevelMin;
    m_LevelMax = data->LevelMax;
    ScriptId = data->scriptId;
    _isBrawl = data->IsBrawl;
    for (int8 i = TEAM_ALLIANCE; i < MAX_TEAMS; ++i)
        m_TeamStartPos[i] = data->TeamStartLoc[i];

    m_maxGroupSize = data->MaxGroupSize;

    uint8 memberCount = 0;
    auto offset = MS::Battlegrounds::QueueOffsets::Battleground;
    if (IsWargame())
        offset = MS::Battlegrounds::QueueOffsets::Wargame;

    SetQueueID(m_TypeID | static_cast<uint64>(memberCount & 0x3F) << 24 | offset);
}

uint32 Battleground::GetMapId() const
{
    return m_MapId;
}

void Battleground::SetBgMap(BattlegroundMap* map)
{
    m_Map = map;
}

BattlegroundMap* Battleground::GetBgMap() const
{
    if (m_Map)
        return m_Map;
    return nullptr;
}

BattlegroundMap* Battleground::FindBgMap() const
{
    return m_Map;
}

inline void Battleground::_ProcessOfflineQueue()
{
    if (!m_OfflineQueue.empty())
    {
        auto itr = _players.find(*m_OfflineQueue.begin());
        if (itr != _players.end())
        {
            if (itr->second.OfflineRemoveTime <= sWorld->GetGameTime())
            {
                RemovePlayerAtLeave(itr->first, true, true);
                m_OfflineQueue.pop_front();
            }
        }
    }
}

inline void Battleground::_ProcessRessurect(uint32 diff)
{
    m_LastResurrectTime += diff;
    uint32 resurrectTime = RESURRECTION_INTERVAL;
    if (IsBrawl())
        resurrectTime = uint32(GetTypeID() == MS::Battlegrounds::BattlegroundTypeId::BrawlBattlegroundHotmoguTemplate ? RESURRECTION_INTERVAL_HOTMOGU : RESURRECTION_INTERVAL_BRAWL);
    
    if (m_LastResurrectTime >= resurrectTime)
    {
        if (GetReviveQueueSize())
        {
            for (auto& itr : m_ReviveQueue)
            {
                Creature* sh = nullptr;
                for (GuidVector::const_iterator itr2 = itr.second.begin(); itr2 != itr.second.end(); ++itr2)
                {
                    auto player = ObjectAccessor::FindPlayer(*itr2);
                    if (!player)
                        continue;

                    if (!sh && player->IsInWorld())
                        if ((sh = player->GetMap()->GetCreature(itr.first)))
                            sh->CastSpell(sh, SPELL_SPIRIT_HEAL, true);

                    player->CastSpell(player, SPELL_RESURRECTION_VISUAL, true);
                    _resurrectQueue.push_back(*itr2);
                }

                itr.second.clear();
            }

            m_ReviveQueue.clear();
            m_LastResurrectTime = 0;
        }
        else
            m_LastResurrectTime = 0;
    }
    else if (m_LastResurrectTime > 500)
    {
        for (GuidVector::const_iterator itr = _resurrectQueue.begin(); itr != _resurrectQueue.end(); ++itr)
        {
            auto player = ObjectAccessor::FindPlayer(*itr);
            if (!player)
                continue;
            player->ResurrectPlayer(1.0f);
            player->CastSpell(player, SPELL_SPIRIT_HEAL_MANA, true);

            if (!player->HasSpell(155228) && !player->HasSpell(205024) && player->GetSpecializationId() != SPEC_MAGE_FIRE &&
                player->GetSpecializationId() != SPEC_MAGE_ARCANE && player->GetSpecializationId() != SPEC_DK_BLOOD && player->GetSpecializationId() != SPEC_DK_FROST)
                player->CastSpell(player, SPELL_PET_SUMMONED, true);

            sObjectAccessor->ConvertCorpseForPlayer(*itr);
        }
        _resurrectQueue.clear();
    }
}

inline void Battleground::_ProcessProgress(uint32 diff)
{
    if (!m_PrematureCountDown)
    {
        m_PrematureCountDown = true;
        m_PrematureCountDownTimer = sBattlegroundMgr->GetPrematureFinishTime();
    }
    else if (m_PrematureCountDownTimer < diff)
    {
        uint32 winner = 0;
        if (GetMaxScore())
        {
            if (GetTeamScore(TEAM_ALLIANCE) != GetTeamScore(TEAM_HORDE))
            {
                winner = GetTeamScore(TEAM_ALLIANCE) > GetTeamScore(TEAM_HORDE) ? ALLIANCE : HORDE;
                if (!IsScoreIncremental())
                    winner = winner == ALLIANCE ? HORDE : ALLIANCE;
            }
            else
                winner = 0;
        }
        else if (GetPlayersCountByTeam(ALLIANCE) >= GetMinPlayersPerTeam())
            winner = ALLIANCE;
        else if (GetPlayersCountByTeam(HORDE) >= GetMinPlayersPerTeam())
            winner = HORDE;

        EndBattleground(winner);
        m_PrematureCountDown = false;
    }
    else if (!sBattlegroundMgr->isTesting())
    {
        uint32 newtime = m_PrematureCountDownTimer - diff;
        if (newtime > MINUTE * IN_MILLISECONDS)
        {
            if (newtime / (MINUTE * IN_MILLISECONDS) != m_PrematureCountDownTimer / (MINUTE * IN_MILLISECONDS))
                PSendMessageToAll(LANG_BATTLEGROUND_PREMATURE_FINISH_WARNING, CHAT_MSG_SYSTEM, nullptr, static_cast<uint32>(m_PrematureCountDownTimer / (MINUTE * IN_MILLISECONDS)));
        }
        else
        {
            if (newtime / (15 * IN_MILLISECONDS) != m_PrematureCountDownTimer / (15 * IN_MILLISECONDS))
                PSendMessageToAll(LANG_BATTLEGROUND_PREMATURE_FINISH_WARNING_SECS, CHAT_MSG_SYSTEM, nullptr, static_cast<uint32>(m_PrematureCountDownTimer / IN_MILLISECONDS));
        }

        m_PrematureCountDownTimer = newtime;
    }
}

inline void Battleground::_ProcessJoin(uint32 diff)
{
    ModifyStartDelayTime(Milliseconds(diff));

    m_EndTime = Minutes(5);

    if (m_ResetStatTimer > 5000)
    {
        m_ResetStatTimer = 0;
        for (const auto& itr : GetPlayers())
            if (Player* player = ObjectAccessor::FindPlayer(itr.first))
                player->ResetAllPowers();
    }

    if (!(m_Events & BG_STARTING_EVENT_1))
    {
        m_Events |= BG_STARTING_EVENT_1;

        if (!FindBgMap())
        {
            TC_LOG_ERROR(LOG_FILTER_BATTLEGROUND, "Battleground::_ProcessJoin: map (map id: %u, instance id: %u) is not created!", m_MapId, m_InstanceID);
            EndNow();
            return;
        }

        if (!SetupBattleground())
        {
            EndNow();
            return;
        }

        StartingEventCloseDoors();
        Milliseconds start_timer = IsArena() ? CalcArenaCountdown(GetJoinType()) : m_messageTimer[BG_STARTING_EVENT_FIRST];

        if (GetMapId() == 1101) // Deathmatch
            start_timer = Seconds(15);

        SetStartDelayTime(start_timer);
        SendBroadcastText(m_broadcastMessages[BG_STARTING_EVENT_FIRST], CHAT_MSG_BG_SYSTEM_NEUTRAL);
    }
    else if (GetStartDelayTime() <= (IsArena() ? m_messageTimer[BG_STARTING_EVENT_SECOND] / 2 : m_messageTimer[BG_STARTING_EVENT_SECOND]) && !(m_Events & BG_STARTING_EVENT_2))
    {
        m_Events |= BG_STARTING_EVENT_2;
        SendBroadcastText(m_broadcastMessages[BG_STARTING_EVENT_SECOND], CHAT_MSG_BG_SYSTEM_NEUTRAL);
    }
    else if (GetStartDelayTime() <= (IsArena() ? m_messageTimer[BG_STARTING_EVENT_THIRD] / 2 : m_messageTimer[BG_STARTING_EVENT_THIRD]) && !(m_Events & BG_STARTING_EVENT_3))
    {
        m_Events |= BG_STARTING_EVENT_3;
        SendBroadcastText(m_broadcastMessages[BG_STARTING_EVENT_THIRD], CHAT_MSG_BG_SYSTEM_NEUTRAL);
    }
    else if (GetStartDelayTime() <= m_messageTimer[BG_STARTING_EVENT_FOURTH] && !(m_Events & BG_STARTING_EVENT_4))
    {
        m_Events |= BG_STARTING_EVENT_4;

        StartingEventOpenDoors();
        SendBroadcastText(m_broadcastMessages[BG_STARTING_EVENT_FOURTH], CHAT_MSG_BG_SYSTEM_NEUTRAL);

        SetStatus(STATUS_IN_PROGRESS);
        SetStartDelayTime(m_messageTimer[BG_STARTING_EVENT_FOURTH]);

        if (!IsArena())
        {
            PlaySoundToAll(BG_SOUND_START);

            for (auto const& itr : GetPlayers())
                if (Player* player = ObjectAccessor::FindPlayer(itr.first))
                {
                    if (sWorld->getBoolConfig(CONFIG_CROSSFACTIONBG) && !IsRBG()) // make some visual 
                    {
                        if (player->GetTeam() != player->GetBGTeam())
                        {
                            player->CastSpell(player, player->GetBGTeam() == TEAM_HORDE ? SPELL_BG_MORPH_FACTION_HORDE : SPELL_BG_MORPH_FACTION_ALLIANCE, true);
                            ChatHandler(player).PSendSysMessage("You are playing for the %s in %s!", player->GetBGTeam() == ALLIANCE ? "|cff0000FFalliance|r" : "|cffFF0000horde|r", GetName());
                        }
                    }

                    player->RemoveAurasDueToSpell(SPELL_BG_PREPARATION);
                    player->ResetAllPowers();
                }

            if (sWorld->getBoolConfig(CONFIG_BATTLEGROUND_QUEUE_ANNOUNCER_ENABLE))
                sWorld->SendWorldText(LANG_BG_STARTED_ANNOUNCE_WORLD, GetName(), GetMinLevel(), GetMaxLevel());
        }

        if (IsArena() || IsRBG())
        {
            for (auto const& itr : GetPlayers())
                if (Player* player = ObjectAccessor::FindPlayer(itr.first))
                {
                    if (auto group = player->GetGroup())
                        if (group->GetLeaderGUID() == player->GetGUID())
                            sBattlegroundMgr->AddSpectatorData(player->GetInstanceId(), player->GetGUID());
                }
        }
    }

    if (GetRemainingTime() > Milliseconds(0) && (m_EndTime -= Milliseconds(diff)) > Milliseconds(0))
        m_EndTime = GetRemainingTime() - Milliseconds(diff);
}

inline void Battleground::_ProcessLeave(uint32 diff)
{
    m_EndTime = GetRemainingTime() - Milliseconds(diff);
    if (GetRemainingTime() <= Milliseconds(0))
    {
        m_EndTime = Milliseconds(0);
        std::map<ObjectGuid, BattlegroundPlayer>::iterator next;
        for (auto itr = _players.begin(); itr != _players.end(); itr = next)
        {
            next = itr;
            ++next;
            RemovePlayerAtLeave(itr->first, true, true);
        }
    }
}

Player* Battleground::GetPlayer(ObjectGuid guid, bool offlineRemove, char const* context)
{
    if (!offlineRemove)
    {
        if (Player* player = ObjectAccessor::FindPlayer(guid))
            return player;

        TC_LOG_ERROR(LOG_FILTER_BATTLEGROUND, "Battleground::%s: player (GUID: %u) not found", context, guid.GetCounter());
    }

    return nullptr;
}

Player* Battleground::GetPlayer(std::pair<ObjectGuid, BattlegroundPlayer> pair, char const* context)
{
    return GetPlayer(pair.first, pair.second.OfflineRemoveTime, context);
}

Player* Battleground::GetPlayerForTeam(uint32 teamId, std::pair<ObjectGuid, BattlegroundPlayer> pair, char const* context)
{
    if (pair.second.Team != teamId)
        return nullptr;

    return GetPlayer(pair, context);
}

Position const* Battleground::GetTeamStartPosition(TeamId teamId)
{
    if (teamId < TEAM_NEUTRAL)
        return &m_TeamStartPos[teamId];
    return &m_TeamStartPos[TEAM_NEUTRAL];
}

void Battleground::SendPacketToAll(WorldPacket const* packet)
{
    for (auto const& itr : GetPlayers())
        if (Player* player = GetPlayer(itr, "SendPacketToAll"))
            player->SendDirectMessage(packet);
}

void Battleground::SendPacketToTeam(uint32 TeamID, WorldPacket const* packet, Player* sender, bool self)
{
    for (auto const& itr : GetPlayers())
        if (Player* player = GetPlayerForTeam(TeamID, itr, "SendPacketToTeam"))
            if (self || sender != player)
                player->SendDirectMessage(packet);
}

void Battleground::PlaySoundToAll(uint32 soundKitID, ObjectGuid sourceGuid /*= ObjectGuid::Empty*/)
{
    SendPacketToAll(WorldPackets::Misc::PlaySound(sourceGuid, soundKitID).Write());
}

void Battleground::SendChatMessage(Creature* source, uint8 textId, WorldObject* target /*= nullptr*/)
{
    sCreatureTextMgr->SendChat(source, textId, target ? target->GetGUID() : ObjectGuid::Empty);
}

void Battleground::PlaySoundToTeam(uint32 soundKitID, uint32 TeamID)
{
    for (auto const& itr : GetPlayers())
        if (Player* player = GetPlayerForTeam(TeamID, itr, "PlaySoundToTeam"))
            player->SendDirectMessage(WorldPackets::Misc::PlaySound(ObjectGuid::Empty, soundKitID).Write());
}

void Battleground::CastSpellOnTeam(uint32 SpellID, uint32 TeamID)
{
    for (auto const& itr : GetPlayers())
        if (Player* player = GetPlayerForTeam(TeamID, itr, "CastSpellOnTeam"))
            player->CastSpell(player, SpellID, true);
}

void Battleground::RemoveAuraOnTeam(uint32 SpellID, uint32 TeamID)
{
    for (auto const& itr : GetPlayers())
        if (Player* player = GetPlayerForTeam(TeamID, itr, "RemoveAuraOnTeam"))
            player->RemoveAura(SpellID);
}

void Battleground::YellToAll(Creature* creature, const char* text, uint32 language)
{
    for (auto const& itr : GetPlayers())
        if (Player* player = GetPlayer(itr, "YellToAll"))
        {
            WorldPackets::Chat::Chat packet;
            packet.Initialize(CHAT_MSG_MONSTER_YELL, static_cast<Language>(language), creature, player, text);
            player->SendDirectMessage(packet.Write());
        }
}

void Battleground::RewardHonorToTeam(uint32 Honor, uint32 TeamID)
{
    for (auto const& itr : GetPlayers())
        if (Player* player = GetPlayerForTeam(TeamID, itr, "RewardHonorToTeam"))
            UpdatePlayerScore(player, SCORE_BONUS_HONOR, Honor);
}

void Battleground::RewardReputationToTeam(uint32 factionIDAlliance, uint32 factionIDHorde, uint32 reputation, uint32 teamID)
{
    if (FactionEntry const* factionEntry = sFactionStore.LookupEntry(teamID == ALLIANCE ? factionIDAlliance : factionIDHorde))
        for (auto const& itr : GetPlayers())
            if (Player* player = GetPlayerForTeam(teamID, itr, "RewardReputationToTeam"))
                player->GetReputationMgr().ModifyReputation(factionEntry, reputation);
}

void Battleground::UpdateWorldState(uint32 variableID, uint32 value, bool hidden /*= false*/)
{
    WorldPackets::WorldState::UpdateWorldState packet;
    packet.Value = value;
    packet.VariableID = static_cast<WorldStates>(variableID);
    packet.Hidden = hidden;
    SendPacketToAll(packet.Write());

    //sWorldStateMgr.SetWorldState(variableID, GetInstanceID(), value, hidden);
}

void Battleground::SendBattleGroundPoints(bool isHorde, int32 teamScores, bool broadcast /*= true*/, Player* player /*= nullptr*/)
{
    WorldPackets::Battleground::Points point;
    point.BgPoints = teamScores;
    point.Team = isHorde;
    if (broadcast)
        SendPacketToAll(point.Write());
    else if (player)
        player->SendDirectMessage(point.Write());
}

void Battleground::EndBattleground(uint32 winner)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    if (HaveSpectators())
        for (auto vGuid : _spectators)
            if (Player* player = ObjectAccessor::FindPlayer(vGuid))
                player->TeleportToBGEntryPoint();

    RemoveFromBGFreeSlotQueue();

    int32 broadcastID = 0;

    uint8 bType = MS::Battlegrounds::GetBracketByJoinType(GetJoinType());

    if (winner == ALLIANCE)
    {
        broadcastID = IsBattleground() ? 7335 : 63033;
        PlaySoundToAll(BG_SOUND_ALLIANCE_WIN);
        SetWinner(WINNER_ALLIANCE);
    }
    else if (winner == HORDE)
    {
        broadcastID = IsBattleground() ? 7336 : 63037;
        PlaySoundToAll(BG_SOUND_HORDE_WIN);
        SetWinner(WINNER_HORDE);
    }
    else
        SetWinner(WINNER_NONE);

    if (IsArena())
        UpdateWorldState(WorldStates::ARENA_SHOW_END_TIMER, 0);

    TC_LOG_DEBUG(LOG_FILTER_BATTLEGROUND, "BATTLEGROUND: EndBattleground. JounType: %u, Bracket %u, Winer %u", GetJoinType(), bType, winner);

    SetStatus(STATUS_WAIT_LEAVE);
    m_EndTime = Minutes(2);

    bool guildAwarded = false;

    for (auto const& itr : GetPlayers())
    {
        Player* player = GetPlayer(itr, "EndBattleground");
        m_allMembers.erase(itr.first);

        if (!player)
        {
           if (IsArena() && IsRated())
           {
               Bracket* bracket = sBracketMgr->TryGetOrCreateBracket(itr.first, bType);
               uint32 team = itr.second.Team;
               uint32 gain = bracket->FinishGame(false, GetMatchmakerRating(team == winner ? GetOtherTeam(winner) : winner));

               _arenaTeamScores[team == winner ? 0 : 1].Assign(bracket->getRating(), bracket->getRating() - gain, bracket->getMMV() - bracket->getLastMMRChange());
           }

           continue;
        }

        if (!player) // Double check, can crashed with player == nullptr
            continue;

        Bracket* bracket = GetJoinType() ? player->getBracket(bType) : nullptr;
        if (player->HasAuraType(SPELL_AURA_SPIRIT_OF_REDEMPTION))
            player->RemoveAurasByType(SPELL_AURA_MOD_SHAPESHIFT);

        if (!player->isAlive())
        {
            player->ResurrectPlayer(1.0f);
            player->SpawnCorpseBones();
        }
        else
        {
            player->CombatStop();
            player->getHostileRefManager().deleteReferences();
        }

        uint32 team = itr.second.Team;

        if (!IsWargame() && IsArena() && IsRated() && winner != WINNER_NONE)
        {
            uint32 gain = bracket->FinishGame(team == winner, GetMatchmakerRating(team == winner ? GetOtherTeam(winner) : winner));

            if (team == winner)
            {
                player->UpdateAchievementCriteria(CRITERIA_TYPE_WIN_RATED_ARENA, 1);
                player->UpdateAchievementCriteria(CRITERIA_TYPE_WIN_ARENA, GetMapId());
                player->KilledMonsterCredit(542183); // for daily event quest
                _arenaTeamScores[0].Assign(bracket->getRating(), bracket->getRating() + gain, bracket->getMMV() - bracket->getLastMMRChange());
            }
            else
            {
                player->GetAchievementMgr()->ResetAchievementCriteria(CRITERIA_TYPE_WIN_RATED_ARENA, CRITERIA_CONDITION_NO_LOSE);
                _arenaTeamScores[1].Assign(bracket->getRating(), bracket->getRating() - gain, bracket->getMMV() - bracket->getLastMMRChange());
            }

            player->UpdateAchievementCriteria(CRITERIA_TYPE_PLAY_ARENA, GetMapId());
        }
        else if (!IsWargame() && IsArena() && IsRated())
        {
            uint16 lossRating = bracket->getRating() >= 16 ? 16 : bracket->getRating();

            _arenaTeamScores[TEAM_ALLIANCE].Assign(bracket->getRating(), bracket->getRating() - lossRating, bracket->getMMV());
            _arenaTeamScores[TEAM_HORDE].Assign(bracket->getRating(), bracket->getRating() - lossRating, bracket->getMMV());

            bracket->FinishGame(false, lossRating, true);
        }

        player->RemoveAura(SPELL_BG_HONORABLE_DEFENDER_25Y);
        player->RemoveAura(SPELL_BG_HONORABLE_DEFENDER_60Y);

        if (!IsWargame() && (player->IsInvitedForBattlegroundQueueType(MS::Battlegrounds::BattlegroundQueueTypeId::BattlegroundRandom) || IsArena() || IsSkirmish() || IsRBG() || IsBG() || IsBrawl()))
            PlayerReward(player, team == winner);

        if (team == winner)
        {
            if (!IsWargame() && !player->HasWinToday(sBattlegroundMgr->GetPvpRewardType(this)))
                player->SetWinToday(true, sBattlegroundMgr->GetPvpRewardType(this), false);

            player->UpdateAchievementCriteria(CRITERIA_TYPE_BE_SPELL_TARGET, 192953); // win spell ?

            if (IsBattleground() && !IsSkirmish())
            {
                player->UpdateAchievementCriteria(CRITERIA_TYPE_WIN_BATTLEGROUND, 1); // general counts
                player->UpdateAchievementCriteria(CRITERIA_TYPE_WIN_BG, GetMapId()); // current bg mapid
            }

            if (IsBrawl())
                player->UpdateAchievementCriteria(CRITERIA_TYPE_WIN_BRAWL, true);

            if (!guildAwarded)
            {
                guildAwarded = true;
                if (Guild* guild = sGuildMgr->GetGuildById(GetBgMap()->GetOwnerGuildId(player->GetTeam())))
                {
                    guild->UpdateAchievementCriteria(CRITERIA_TYPE_WIN_BATTLEGROUND, 1, 0, 0, nullptr, player); // what need? current or general ????
                    if (IsArena() && IsRated() && winner != WINNER_NONE)
                    {
                        ASSERT(bracket);
                        guild->UpdateAchievementCriteria(CRITERIA_TYPE_WIN_RATED_ARENA, std::max<uint32>(bracket->getRating(), 1), 0, 0, nullptr, player);
                    }
                }
            }
        }

        player->ResetAllPowers();
        player->CombatStopWithPets(true);

        BlockMovement(player);

        if (IsRBG())
        {
            player->getBracket(MS::Battlegrounds::BracketType::RatedBattleground)->FinishGame(team == winner, GetMatchmakerRating(team == winner ? GetOtherTeam(winner) : winner));

            if (bracket)
                player->UpdateAchievementCriteria(CRITERIA_TYPE_REACH_RBG_RATING, std::max<uint32>(bracket->getRating(), 1));

            if (player->GetGroup() && player->GetGroup()->IsGuildGroup(player->GetGuildGUID(), true, true))
                if (auto guild = player->GetGuild())
                    guild->CompleteGuildChallenge(ChallengeRatedBG);
        }

        auto queueTypeID = MS::Battlegrounds::GetBgQueueTypeIdByBgTypeID(GetTypeID(), GetJoinType());

        WorldPackets::Battleground::BattlefieldStatusActive battlefieldStatus;
        sBattlegroundMgr->BuildBattlegroundStatusActive(&battlefieldStatus, this, player, player->GetBattlegroundQueueIndex(queueTypeID), player->GetBattlegroundQueueJoinTime(queueTypeID), GetJoinType());
        player->SendDirectMessage(battlefieldStatus.Write());

        if (!IsWargame())
            player->UpdateAchievementCriteria(CRITERIA_TYPE_COMPLETE_BATTLEGROUND, 1);
    }

    if (IsArena() && IsRated())
    {
       for (auto itr : m_allMembers)
       {
           Bracket* bracket = sBracketMgr->TryGetOrCreateBracket(itr.first, bType);
           uint32 team = itr.second;
           uint32 gain = bracket->FinishGame(false, GetMatchmakerRating(team == winner ? GetOtherTeam(winner) : winner));

           _arenaTeamScores[team == winner ? 0 : 1].Assign(bracket->getRating(), bracket->getRating() - gain, bracket->getMMV() - bracket->getLastMMRChange());
       }
    }

    if (IsArena() && GetJoinType() == MS::Battlegrounds::JoinType::Arena1v1)
    {
        for (auto const& playerItr : GetPlayers())
        {
            Player* player = GetPlayer(playerItr, "EndBattleground");
            uint32 team = playerItr.second.Team;
            if (!player)
                continue;

            auto playerScore = PlayerScores.find(playerItr.first);
            if (playerScore != PlayerScores.end())
            {
                player->ModifyDeathMatchStats(playerScore->second->GetScore(SCORE_KILLING_BLOWS), playerScore->second->GetScore(SCORE_DEATHS), playerScore->second->GetScore(SCORE_DAMAGE_DONE),
                    BattlegroundDeathMatch::CalculateRating(playerScore->second), playerScore->second->GetScore(SCORE_KILLING_BLOWS));
            }
            else
            {
                uint8 killingBlows = team == winner;
                uint8 deaths = !killingBlows;
                int64 damage = player->GetMaxHealth();
                player->ModifyDeathMatchStats(killingBlows, deaths, damage,
                    BattlegroundDeathMatch::CalculateRating(killingBlows, deaths, damage), killingBlows);
            }

            player->DeMorph();
            player->ResetCustomDisplayId();
            player->SetObjectScale(1.0f);
        }
    }

    WorldPackets::Battleground::PVPLogData pvpLogData;
    BuildPvPLogDataPacket(pvpLogData);
    SendPacketToAll(pvpLogData.Write());

    SendBroadcastText(broadcastID, CHAT_MSG_BG_SYSTEM_NEUTRAL);
}

bool Battleground::HandleArenaLogPlayerNames(std::string & Team1, std::string & Team2, std::string Marks, uint32 winner)
{
    bool correction = true;

    if (!winner)
    {
        uint8 IPSimilarity = 1;
        uint8 HWIdSimilarity = 1;
        bool findSameIP = false;
        bool findSameHWID = false;

        for (auto& itr : m_nameList[ALLIANCE])
        {
            if (itr.IPMark)
                continue;

            for (auto& j : m_nameList[ALLIANCE])
            {
                if (j.IPMark || ((1 << j.PlayerID) & itr.PlayersCheckedMask))
                    continue;

                j.PlayersCheckedMask |= (1 << itr.PlayerID);
                itr.PlayersCheckedMask |= (1 << j.PlayerID);

                if (j.PlayerIP == itr.PlayerIP)
                {
                    findSameIP = true;
                    j.IPMark = IPSimilarity;
                    itr.IPMark = IPSimilarity;

                    if (j.HWId && itr.HWId && j.HWId == itr.HWId)
                    {
                        findSameHWID = true;
                        j.HWIdMark = HWIdSimilarity;
                        itr.HWIdMark = HWIdSimilarity;
                    }
                }
            }

            for (auto& j : m_nameList[HORDE])
            {
                if (j.IPMark || ((1 << j.PlayerID) & itr.PlayersCheckedMask))
                    continue;

                j.PlayersCheckedMask |= (1 << itr.PlayerID);
                itr.PlayersCheckedMask |= (1 << j.PlayerID);

                if (j.PlayerIP == itr.PlayerIP)
                {
                    findSameIP = true;
                    j.IPMark = IPSimilarity;
                    itr.IPMark = IPSimilarity;

                    if (j.HWId && itr.HWId && j.HWId == itr.HWId)
                    {
                        findSameHWID = true;
                        j.HWIdMark = HWIdSimilarity;
                        itr.HWIdMark = HWIdSimilarity;
                    }
                }
            }

            if (findSameIP)
            {
                findSameIP = false;
                IPSimilarity++;
            }

            if (findSameHWID)
            {
                findSameHWID = false;
                HWIdSimilarity++;
            }
        }

        for (auto& itr : m_nameList[HORDE])
        {
            if (itr.IPMark)
                continue;

            for (auto& j : m_nameList[HORDE])
            {
                if (j.IPMark || ((1 << j.PlayerID) & itr.PlayersCheckedMask))
                    continue;

                j.PlayersCheckedMask |= (1 << itr.PlayerID);
                itr.PlayersCheckedMask |= (1 << j.PlayerID);

                if (j.PlayerIP == itr.PlayerIP)
                {
                    findSameIP = true;
                    j.IPMark = IPSimilarity;
                    itr.IPMark = IPSimilarity;

                    if (j.HWId && itr.HWId && j.HWId == itr.HWId)
                    {
                        findSameHWID = true;
                        j.HWIdMark = HWIdSimilarity;
                        itr.HWIdMark = HWIdSimilarity;
                    }
                }
            }

            if (findSameIP)
            {
                findSameIP = false;
                IPSimilarity++;
            }

            if (findSameHWID)
            {
                findSameHWID = false;
                HWIdSimilarity++;
            }
        }

        for (auto const& itr : m_nameList[ALLIANCE])
        {
            if (!correction)
                Team1 += ", ";

            correction = false;

            if (itr.HWIdMark)
                Team1 += Marks[itr.HWIdMark - 1];

            Team1 += itr.PlayerName;
        }

        correction = true;

        for (auto const& itr : m_nameList[HORDE])
        {
            if (!correction)
                Team2 += ", ";

            correction = false;
            
            if (itr.HWIdMark)
                Team2 += Marks[itr.HWIdMark - 1];

            Team2 += itr.PlayerName;
        }
    }
    else
    {
        bool attention = false;

        for (auto const& itr : m_nameList[winner])
        {
            if (!correction)
                Team1 += ", ";

            correction = false;

            if (itr.IPMark)
            {
                Team1 += Marks[itr.IPMark - 1];

                if (itr.HWIdMark)
                    attention = true;
            }

            Team1 += itr.PlayerName;
        }

        correction = true;

        for (auto const& itr : m_nameList[GetOtherTeam(winner)])
        {
            if (!correction)
                Team2 += ", ";

            correction = false;

            if (itr.IPMark)
            {
                Team2 += Marks[itr.IPMark - 1];

                if (itr.HWIdMark)
                    attention = true;
            }

            Team2 += itr.PlayerName;
        }

        return attention;
    }

    return false;
}

void Battleground::PlayerReward(Player* player, bool isWinner)
{
    if (!player)
        return;

    PvpRewardTypes type = sBattlegroundMgr->GetPvpRewardType(this);
    PvpReward* reward = sBattlegroundMgr->GetPvpReward(type);
    if (!reward)
        return;

    if (GetMinLevel() < reward->BracketLevel)
        return;

    uint32 playerSpecID = player->GetLootSpecID();
    uint8 playerLevel = player->getLevel();
    bool isAlliance = player->GetTeam() == ALLIANCE;
    uint32 artifactRewardItem = reward->ItemCALose;
    uint32 questId = isAlliance ? reward->QuestAlose : reward->QuestHLose;

    if (isWinner)
    {
        if (!player->HasWinToday(type))
        {
            artifactRewardItem = reward->ItemCAFirst;
            questId = isAlliance ? reward->QuestAFirst : reward->QuestHFirst;
        }
        else
        {
            artifactRewardItem = reward->ItemCAWin;
            questId = isAlliance ? reward->QuestAWin : reward->QuestHWin;
        }
    }

    if (Quest const* quest = sQuestDataStore->GetQuestTemplate(questId))
        player->RewardQuest(quest, 0, nullptr, false);

    if (artifactRewardItem)
    {
        ItemPosCountVec dest;
        if (player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, artifactRewardItem, 1) == EQUIP_ERR_OK)
        {
            if (auto item = player->StoreNewItem(dest, artifactRewardItem, true))
            {
                player->SendNewItem(item, 1, true, false, true);
                //player->SendDisplayToast(artifactRewardItem, ToastType::ITEM, false, 1, IsRated() ? DisplayToastMethod::DISPLAY_TOAST_ENTRY_RATED_PVP_REWARD : DisplayToastMethod::DISPLAY_TOAST_ENTRY_PVP_FACTION, 0, item);
            }
        }
    }

    if (!isWinner) // Need calculate for active users
        return;

    if (roll_chance_f(reward->ChestChance))
    {
        uint32 chestId = isAlliance ? reward->ChestA : reward->ChestH;
        ItemPosCountVec dest;
        if (player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, chestId, 1) == EQUIP_ERR_OK)
        {
            if (auto item = player->StoreNewItem(dest, chestId, true))
            {
                player->SendNewItem(item, 1, true, false, true);
                //player->SendDisplayToast(chestId, ToastType::ITEM, false, 1, IsRated() ? DisplayToastMethod::DISPLAY_TOAST_ENTRY_RATED_PVP_REWARD : DisplayToastMethod::DISPLAY_TOAST_ENTRY_PVP_FACTION, 0, item);
            }
        }
    }
    float chance = player->CalculateLegendaryDropChance(reward->RateLegendary);
    if (roll_chance_f(chance))
    {
        uint32 legendaryItemID = sObjectMgr->GetRandomLegendaryItem(player);
        if (legendaryItemID)
        {
            ItemPosCountVec dest;
            if (player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, legendaryItemID, 1) == EQUIP_ERR_OK)
            {
                if (auto item = player->StoreNewItem(dest, legendaryItemID, true, Item::GenerateItemRandomPropertyId(legendaryItemID, playerSpecID), GuidSet(), sObjectMgr->GetItemBonusTree(legendaryItemID, 0, playerLevel, 0, 0)))
                {
                    sLog->outWarden("Player %s on map %u with killpoints %f and chance %f looted legendary item %u from source: PvP (rewardType %u)", player->GetName(), player->GetMapId(), player->m_killPoints, chance, legendaryItemID, uint8(type));
                    player->SendNewItem(item, 1, true, false, true);
                    player->SendDisplayToast(legendaryItemID, ToastType::ITEM, false, 1, DisplayToastMethod::DISPLAY_TOAST_ITEM_LEGENDARY, 0, item);
                    player->m_killPoints = 0.0f;
                    item->dungeonEncounterID = 1;
                    sLog->outArena(0, "PlayerReward: player (%s) (%s) legendaryItemID %u PvPtype %u", player->GetName(), player->GetGUID().ToString().c_str(), legendaryItemID, type);
                }
            }
        }
    }

    if ((IsArena() || IsRBG()) && !IsSkirmish())
    {
        uint32 obliterumForgeQuestItem = isAlliance ? 146975 : 147417;
        uint32 obliterumForgeQuest = isAlliance ? 46810 : 46946;
        if (!player->GetQuestRewardStatus(obliterumForgeQuest) && !player->GetItemCount(obliterumForgeQuestItem, true))
        {
            ItemPosCountVec dest;
            if (player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, obliterumForgeQuestItem, 1) == EQUIP_ERR_OK)
            {
                if (auto item = player->StoreNewItem(dest, obliterumForgeQuestItem, true))
                {
                    player->SendNewItem(item, 1, true, false, true);
                    player->SendDisplayToast(obliterumForgeQuestItem, ToastType::ITEM, false, 1, IsRated() ? DisplayToastMethod::DISPLAY_TOAST_ENTRY_RATED_PVP_REWARD : DisplayToastMethod::DISPLAY_TOAST_ENTRY_PVP_FACTION, 0, item);
                }
            }
        }
    }

    bool canLootItem = roll_chance_f(reward->ItemsChance);
    // sLog->outArena("PlayerReward: player (%s) (%s) canLootItem %u ItemsChance %f", player->GetName(), player->GetGUID().ToString().c_str(), canLootItem, reward->ItemsChance);
    if (!canLootItem) // Roll can take item
        return;

    uint32 itemId = 0;
    uint32 relicId = 0;
    uint32 rating = 0;

    std::vector<uint32> itemsContainer = isAlliance ? reward->ItemsA : reward->ItemsH;

    uint32 needLevel = reward->BaseLevel;

    if ((IsArena() || IsRBG()) && !IsSkirmish())
        player->GetPvPRatingAndLevel(reward, type, rating, needLevel, true);

    if ((IsBattleground() || IsSkirmish()) && roll_chance_f(reward->ChanceBonusLevel))
        needLevel += reward->BonusBaseLevel;

    sLog->outArena(0, "PlayerReward: player (%s) (%s) itemId %u PvPtype %u", player->GetName(), player->GetGUID().ToString().c_str(), itemId, type);

    std::vector<uint32> itemModifiers = player->GetPvPRewardItem(itemId, type, rating, (IsArena() || IsRBG()) && !IsSkirmish(), needLevel);
    if (itemId)
    {
        ItemPosCountVec dest;
        if (player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemId, 1) == EQUIP_ERR_OK)
        {
            if (auto item = player->StoreNewItem(dest, itemId, true, Item::GenerateItemRandomPropertyId(itemId, playerSpecID), GuidSet(), itemModifiers))
            {
                player->SendNewItem(item, 1, true, false, true);
                player->SendDisplayToast(itemId, ToastType::ITEM, false, 1, IsRated() ? DisplayToastMethod::DISPLAY_TOAST_ENTRY_RATED_PVP_REWARD : DisplayToastMethod::DISPLAY_TOAST_ENTRY_PVP_FACTION, 0, item);
            }
        }
    }
}

uint32 Battleground::GetBonusHonorFromKill(uint32 kills) const
{
    return Trinity::Honor::hk_honor_at_level(float(kills));
}

void Battleground::BlockMovement(Player* player)
{
    player->SetClientControl(player, false); // movement disabled NOTE: the effect will be automatically removed by client when the player is teleported from the battleground, so no need to send with uint8(1) in RemovePlayerAtLeave()
}

void Battleground::RemovePlayerAtLeave(ObjectGuid guid, bool Transport, bool SendPacket)
{
    uint32 team = GetPlayerTeam(guid);
    bool participant = false;

    auto itr = _players.find(guid);
    if (itr != _players.end())
    {
        UpdatePlayersCountByTeam(team, true);
        _players.erase(itr);
        participant = true;
    }

    auto itr2 = PlayerScores.find(guid);
    if (itr2 != PlayerScores.end())
    {
        delete itr2->second;
        PlayerScores.erase(itr2);
    }

    RemovePlayerFromResurrectQueue(guid);

    Player* player = ObjectAccessor::FindPlayer(guid);
    if (player)
    {
        if (player->HasAuraType(SPELL_AURA_SPIRIT_OF_REDEMPTION))
            player->RemoveAurasByType(SPELL_AURA_MOD_SHAPESHIFT);

        if (!player->isAlive())
        {
            player->ResurrectPlayer(1.0f);
            player->SpawnCorpseBones();
        }

        if (IsRBG())
            player->RemoveAurasDueToSpell(SPELL_RATED_PVP_TRANSFORM_SUPPRESSION);

        player->CastSpell(player, 248473); ///< Battle sharpness - Able to adjust talents.
        player->ForceChangeTalentGroup(player->GetLastActiveSpec(false));

        if (IsArena() || IsRBG())
        {
            if (auto const& group = player->GetGroup())
                if (group->GetLeaderGUID() == guid)
                    sBattlegroundMgr->EraseSpectatorData(player->GetInstanceId());
        }
    }

    RemovePlayer(player, guid, team);

    uint16 bgTypeId;

    if (player && player->IsInvitedForBattlegroundQueueType(MS::Battlegrounds::BattlegroundQueueTypeId::BattlegroundRandom))
        bgTypeId = MS::Battlegrounds::BattlegroundTypeId::BattlegroundRandom;
    else
        bgTypeId = GetTypeID(!IsRBG());

    uint8 bgQueueTypeId = MS::Battlegrounds::GetBgQueueTypeIdByBgTypeID(bgTypeId, GetJoinType());

    if (participant)
    {
        if (player)
        {
            player->ClearAfkReports();

            if (!team)
                team = player->GetBGTeam();

            if (IsArena() || IsRBG())
            {
                bgTypeId = IsArena() ? MS::Battlegrounds::BattlegroundTypeId::ArenaAll : MS::Battlegrounds::BattlegroundTypeId::RatedBattleground;
                player->RemovePet(nullptr);
                player->ResummonPetTemporaryUnSummonedIfAny();

                if (IsRated() && GetStatus() == STATUS_IN_PROGRESS)
                {
                    Bracket* bracket = player->getBracket(MS::Battlegrounds::GetBracketByJoinType(GetJoinType()));
                    ASSERT(bracket);
                    bracket->FinishGame(false/*lost*/, GetMatchmakerRating(GetOtherTeam(team)));
                }
            }
            if (SendPacket)
            {
                WorldPackets::Battleground::BattlefieldStatusNone battlefieldStatus;
                sBattlegroundMgr->BuildBattlegroundStatusNone(&battlefieldStatus, player, player->GetBattlegroundQueueIndex(bgQueueTypeId), player->GetBattlegroundQueueJoinTime(bgQueueTypeId));
                player->SendDirectMessage(battlefieldStatus.Write());
            }

            player->RemoveBattlegroundQueueId(bgQueueTypeId);
        }
        else
        {
            if (IsRated() && GetStatus() == STATUS_IN_PROGRESS)
            {
                Bracket* bracket = sBracketMgr->TryGetOrCreateBracket(guid, MS::Battlegrounds::GetBracketByJoinType(GetJoinType()));
                ASSERT(bracket);
                bracket->FinishGame(false/*lost*/, GetMatchmakerRating(GetOtherTeam(team)));
            }
        }

        if (Group* group = GetBgRaid(team))
            if (!group->RemoveMember(guid, true))
                SetBgRaid(team, nullptr);

        if (!IsBrawl() || (IsBrawl() && !IsArena() && GetTypeID() != MS::Battlegrounds::BattlegroundTypeId::BrawlArenaAll)) // if it is Brawl arena and smth leave - not decrease size or it can work wrong
            DecreaseInvitedCount(team);

        if (IsBattleground() && !IsRBG() && !IsWargame() && GetStatus() < STATUS_WAIT_LEAVE)
        {
            AddToBGFreeSlotQueue();
            sBattlegroundMgr->ScheduleQueueUpdate(new QueueSchedulerItem(0, 0, bgQueueTypeId, bgTypeId, GetBracketId()));
        }

        SendPacketToTeam(team, WorldPackets::Battleground::PlayerLeft(guid).Write(), player, false);
    }

    if (player)
    {
        player->SetBattlegroundId(0, MS::Battlegrounds::BattlegroundTypeId::None);
        player->SetBGTeam(0);
        player->SetByteValue(PLAYER_FIELD_BYTES_6, PLAYER_BYTES_6_OFFSET_ARENA_FACTION, 0);

        if (Transport)
            player->TeleportToBGEntryPoint();

        player->SetEffectiveLevel(0);

        TC_LOG_INFO(LOG_FILTER_BATTLEGROUND, "BATTLEGROUND: Removed player %s from Battleground.", player->GetName());
    }
}

void Battleground::Reset()
{
    SetWinner(WINNER_NONE);
    SetStatus(STATUS_WAIT_QUEUE);
    m_StartTime = Milliseconds(0);
    m_EndTime = Milliseconds(0);
    m_LastResurrectTime = 0;
    SetJoinType(0);
    SetRated(false);
    _useTournamentRules = false;
    _isSkirmish = false;
    _isWargame = false;
    _isBrawl = false;

    m_Events = 0;

    if (m_InvitedAlliance > 0 || m_InvitedHorde > 0)
        TC_LOG_ERROR(LOG_FILTER_BATTLEGROUND, "Battleground::Reset: one of the counters is not 0 (alliance: %u, horde: %u) for BG (map: %u, instance id: %u)!",
            m_InvitedAlliance, m_InvitedHorde, m_MapId, m_InstanceID);

    m_InvitedAlliance = 0;
    m_InvitedHorde = 0;
    m_InBGFreeSlotQueue = false;

    _players.clear();

    for (auto v : PlayerScores)
        delete v.second;
    PlayerScores.clear();

    for (uint8 i = TEAM_ALLIANCE; i < MAX_TEAMS; ++i)
        _arenaTeamScores[i].Reset();

    ResetBGSubclass();
}

void Battleground::RelocateDeadPlayers(ObjectGuid guideGuid)
{
    GuidVector& ghostList = m_ReviveQueue[guideGuid];
    if (!ghostList.empty())
    {
        for (auto const& v : ghostList)
            if (Player* player = ObjectAccessor::FindPlayer(v))
                if (WorldSafeLocsEntry const* closestGrave = GetClosestGraveYard(player))
                    player->SafeTeleport(GetMapId(), closestGrave->Loc.X, closestGrave->Loc.Y, closestGrave->Loc.Z, player->GetOrientation());

        ghostList.clear();
    }
}

void Battleground::StartBattleground()
{
    m_StartTime = Milliseconds(0);
    m_LastResurrectTime = 0;
    AddToBGFreeSlotQueue();

    sBattlegroundMgr->AddBattleground(GetInstanceID(), GetTypeID(), this);
}

void Battleground::AddPlayer(Player* player)
{
    if (player->HasFlag(PLAYER_FIELD_PLAYER_FLAGS, PLAYER_FLAGS_AFK))
        player->ToggleAFK();

    ObjectGuid guid = player->GetGUID();
    uint32 team = player->GetBGTeam();

    player->SaveLastSpecialization(false);
    if (!IsArena() || IsSkirmish())
        player->ChangeSpecializationForBGIfNeed(GetBracketId());

    BattlegroundPlayer bp;
    bp.Team = team;
    bp.ActiveSpec = player->GetUInt32Value(PLAYER_FIELD_CURRENT_SPEC_ID);
    bp.PrestigeRank = player->GetPrestigeLevel();
    bp.Role = player->GetSpecializationRoleMaskForGroup();
    if (auto const& spec = sChrSpecializationStore[bp.ActiveSpec])
        bp.PrimaryTalentTreeNameIndex = spec->ID;
    _players[guid] = bp;

    UpdatePlayersCountByTeam(team, false);
    SendPacketToTeam(team, WorldPackets::Battleground::PlayerJoined(guid).Write(), player, false);

    player->SetEffectiveLevel(GetMaxLevel());
    player->Dismount();
    player->RemoveAurasByType(SPELL_AURA_MOUNTED);
    player->RemoveAurasByType(SPELL_AURA_FLY);
    // TODO: rewrite it, remove fucking clone auras
    player->RemoveAurasByType(SPELL_AURA_CLONE_CASTER);

    if (IsRBG())
        player->CastSpell(player, SPELL_RATED_PVP_TRANSFORM_SUPPRESSION);

    if (GetStatus() == STATUS_WAIT_JOIN)
    {
        if (!IsArena())
        {
            player->CastSpell(player, SPELL_BG_PREPARATION, true);
        }
        else
        {
            player->RemoveArenaSpellCooldowns(true);
        }
    }

    PlayerAddedToBGCheckIfBGIsRunning(player);
    AddOrSetPlayerToCorrectBgGroup(player, team);

    player->ScheduleDelayedOperation(DELAYED_UPDATE_AFTER_TO_BG);

    player->ResetCustomDisplayId();

    if (IsBrawl())
    {
        if (!player->IsQuestRewarded(47148))
            if (Quest const* qInfo = sQuestDataStore->GetQuestTemplate(47148))
                if (player->CanTakeQuest(qInfo, false) && player->CanAddQuest(qInfo, true))
                    player->AddQuest(qInfo, NULL);
    }
}

void Battleground::OnPlayerEnter(Player* player)
{
    if (!player->IsSpectator())
    {
        if (IsBattleground() || IsSkirmish() || GetJoinType() == MS::Battlegrounds::JoinType::ArenaSoloQ3v3)
        {
            if (!player->HasAura(SPELL_BG_SET_FACTION_HORDE) && player->GetTeam() == ALLIANCE && player->GetBGTeam() != ALLIANCE)
                player->CastSpell(player, SPELL_BG_SET_FACTION_HORDE, true);
            else if (!player->HasAura(SPELL_BG_SET_FACTION_ALLIANCE) && player->GetTeam() == HORDE && player->GetBGTeam() != HORDE)
                player->CastSpell(player, SPELL_BG_SET_FACTION_ALLIANCE, true);
        }

        if (sWorld->getBoolConfig(CONFIG_CROSSFACTIONBG) && IsBattleground() && !IsRBG()) 
        {
            if (player->GetTeam() != player->GetBGTeam())
            {
                player->CastSpell(player, player->GetBGTeam() == TEAM_HORDE ? SPELL_BG_MORPH_FACTION_HORDE : SPELL_BG_MORPH_FACTION_ALLIANCE, true);
                ChatHandler(player).PSendSysMessage("You are playing for the %s in %s!", player->GetBGTeam() == ALLIANCE ? "|cff0000FFalliance|r" : "|cffFF0000horde|r", GetName());
            }
        }

        player->SetByteValue(PLAYER_FIELD_BYTES_6, PLAYER_BYTES_6_OFFSET_ARENA_FACTION, player->GetBGTeam() == HORDE ? 0 : 1);
    }
}

void Battleground::AddMember(ObjectGuid guid, uint32 team)
{
    m_allMembers[guid] = team;
}

void Battleground::AddOrSetPlayerToCorrectBgGroup(Player* player, uint32 team)
{
    ObjectGuid playerGuid = player->GetGUID();
    Group* group = GetBgRaid(team);
    if (!group)
    {
        group = new Group;
        SetBgRaid(team, group);
        group->Create(player);
        sGroupMgr->AddGroup(group);
    }
    else
    {
        if (group->IsMember(playerGuid))
            player->SetBattlegroundOrBattlefieldRaid(group, group->GetMemberGroup(playerGuid));
        else
        {
            group->AddMember(player);
            if (Group* originalGroup = player->GetOriginalGroup())
                if (originalGroup->IsLeader(playerGuid))
                {
                    group->ChangeLeader(playerGuid);
                    group->SendUpdate();
                }
        }
    }
}

void Battleground::UpdateCapturePoint(uint8 type, TeamId teamID, GameObject* node, Player const* player /*= nullptr*/, bool initial /*= false*/, bool forseCustomAnim /*= false*/)
{
    auto pointInfo = node->GetGOInfo()->capturePoint;
    uint32 const spellVisualArray[] = { pointInfo.SpellVisual1, pointInfo.SpellVisual2, pointInfo.SpellVisual3, pointInfo.SpellVisual4, pointInfo.SpellVisual5 };
    auto nodeState = NODE_STATE_NONE;
    auto broadcastID = 0, kitID = 0, visualID = 0;

    switch (type)
    {
        case NODE_STATUS_NEUTRAL:
            nodeState = NODE_STATE_NEUTRAL;
            kitID = 1;
            visualID = spellVisualArray[0];
            break;
        case NODE_STATUS_ASSAULT:
            nodeState = teamID == TEAM_ALLIANCE ? NODE_STATE_ALLIANCE_ASSAULT : NODE_STATE_HORDE_ASSAULT;
            broadcastID = teamID == TEAM_ALLIANCE ? pointInfo.AssaultBroadcastAlliance : pointInfo.AssaultBroadcastHorde;
            kitID = 2;
            visualID = spellVisualArray[teamID == TEAM_ALLIANCE ? 2 : 1];
            break;
        case NODE_STATUS_CAPTURE:
            nodeState = teamID == TEAM_ALLIANCE ? NODE_STATE_ALLIANCE_CAPTURE : NODE_STATE_HORDE_CAPTURE;
            broadcastID = teamID == TEAM_ALLIANCE ? pointInfo.CaptureBroadcastAlliance : pointInfo.CaptureBroadcastHorde;
            kitID = teamID == TEAM_ALLIANCE ? 4 : 3;
            visualID = spellVisualArray[teamID == TEAM_ALLIANCE ? 4 : 3];
            break;
        default:
            break;
    }

    if (forseCustomAnim)
        node->SendCustomAnim(kitID, false);

    if (!initial && !forseCustomAnim)
    {
        node->SendCustomAnim(kitID, false);

        WorldPackets::GameObject::GameObjectPlaySpellVisual objectSpellVisual;
        objectSpellVisual.ObjectGUID = node->GetGUID();
        objectSpellVisual.SpellVisualID = visualID;
        SendPacketToAll(objectSpellVisual.Write());

        auto lang = CHAT_MSG_BG_SYSTEM_NEUTRAL;
        switch (teamID)
        {
            case TEAM_ALLIANCE:
                lang = CHAT_MSG_BG_SYSTEM_ALLIANCE;
                break;
            case TEAM_HORDE:
                lang = CHAT_MSG_BG_SYSTEM_HORDE;
                break;
            default:
                break;
        }

        PlayeCapturePointSound(type, teamID);
        SendBroadcastText(broadcastID, lang, player);
    }

    WorldPackets::Battleground::BattlegroundCapturePointInfo info;
    info.Info.Guid = node->GetGUID();
    info.Info.Pos = node->GetPosition();
    info.Info.NodeState = nodeState;
    if (nodeState == NODE_STATE_HORDE_ASSAULT || nodeState == NODE_STATE_ALLIANCE_ASSAULT)
    {
        info.Info.CaptureTime = pointInfo.CaptureTime - 1;
        info.Info.CaptureTotalDuration = pointInfo.CaptureTime;
    }
    SendPacketToAll(info.Write());

    if (auto worldState = pointInfo.worldState1)
        UpdateWorldState(worldState, nodeState);

    node->SetUInt32Value(GAMEOBJECT_FIELD_SPELL_VISUAL_ID, visualID);
}

void Battleground::PlayeCapturePointSound(uint8 type, TeamId teamID)
{
    switch (type)
    {
        case NODE_STATUS_NEUTRAL:
            PlaySoundToAll(teamID == TEAM_HORDE ? BG_SOUND_FLAG_PLACED_HORDE : BG_SOUND_FLAG_PLACED_ALLIANCE);
            break;
        case NODE_STATUS_ASSAULT:
            PlaySoundToAll(teamID == TEAM_HORDE ? BG_SOUND_CAPTURE_POINT_ASSAULT_HORDE : BG_SOUND_CAPTURE_POINT_ASSAULT_ALLIANCE);
            break;
        case NODE_STATUS_CAPTURE:
            PlaySoundToAll(teamID == TEAM_HORDE ? BG_SOUND_CAPTURE_POINT_CAPTURED_HORDE : BG_SOUND_CAPTURE_POINT_CAPTURED_ALLIANCE);
            break;
        default:
            break;
    }
}

void Battleground::EventPlayerLoggedIn(Player* player)
{
    ObjectGuid guid = player->GetGUID();
    for (auto itr = m_OfflineQueue.begin(); itr != m_OfflineQueue.end(); ++itr)
    {
        if (*itr == guid)
        {
            m_OfflineQueue.erase(itr);
            break;
        }
    }
    _players[guid].OfflineRemoveTime = 0;
    PlayerAddedToBGCheckIfBGIsRunning(player);
}

void Battleground::EventPlayerLoggedOut(Player* player)
{
    ObjectGuid guid = player->GetGUID();
    if (!IsPlayerInBattleground(guid))
        return;

    m_OfflineQueue.push_back(player->GetGUID());
    _players[guid].OfflineRemoveTime = sWorld->GetGameTime() + MAX_OFFLINE_TIME;
    if (GetStatus() == STATUS_IN_PROGRESS && !player->IsSpectator())
    {
        RemovePlayer(player, guid, GetPlayerTeam(guid));

        if (IsArena())
            if (!GetAlivePlayersCountByTeam(player->GetBGTeam()) && GetPlayersCountByTeam(GetOtherTeam(player->GetBGTeam())))
                EndBattleground(GetOtherTeam(player->GetBGTeam()));
    }
}

void Battleground::AddToBGFreeSlotQueue()
{
    if (!m_InBGFreeSlotQueue && IsBattleground() && !IsRBG() /*&& !IsBrawl()*/ && !IsWargame())
    {
        sBattlegroundMgr->BGFreeSlotQueue.push_front(this);
        m_InBGFreeSlotQueue = true;
    }
}

void Battleground::RemoveFromBGFreeSlotQueue()
{
    m_InBGFreeSlotQueue = false;
    for (auto itr = sBattlegroundMgr->BGFreeSlotQueue.begin(); itr != sBattlegroundMgr->BGFreeSlotQueue.end(); ++itr)
        if ((*itr)->GetInstanceID() == m_InstanceID)
        {
            sBattlegroundMgr->BGFreeSlotQueue.erase(itr);
            return;
        }
}

void Battleground::DecreaseInvitedCount(uint32 team)
{
    team == ALLIANCE ? --m_InvitedAlliance : --m_InvitedHorde;
}

void Battleground::IncreaseInvitedCount(uint32 team)
{
    team == ALLIANCE ? ++m_InvitedAlliance : ++m_InvitedHorde;
}

uint32 Battleground::GetInvitedCount(uint32 team) const
{
    return team == ALLIANCE ? m_InvitedAlliance : m_InvitedHorde;
}

uint32 Battleground::GetFreeSlotsForTeam(uint32 Team) const
{
    if (GetStatus() == STATUS_WAIT_JOIN)
        return GetInvitedCount(Team) < GetMaxPlayersPerTeam() ? GetMaxPlayersPerTeam() - GetInvitedCount(Team) : 0;

    uint32 otherTeam = GetInvitedCount(Team == ALLIANCE ? HORDE : ALLIANCE);
    uint32 otherIn = GetPlayersCountByTeam(Team == ALLIANCE ? HORDE : ALLIANCE);

    if (GetStatus() == STATUS_IN_PROGRESS)
    {
        uint32 diff = 0;
        if (otherTeam == GetInvitedCount(Team))
            diff = 1;
        else if (otherTeam > GetInvitedCount(Team))
            diff = otherTeam - GetInvitedCount(Team);

        uint32 diff2 = GetInvitedCount(Team) < GetMaxPlayersPerTeam() ? GetMaxPlayersPerTeam() - GetInvitedCount(Team) : 0;
        uint32 diff3 = 0;
        if (otherIn == GetPlayersCountByTeam(Team))
            diff3 = 1;
        else if (otherIn > GetPlayersCountByTeam(Team))
            diff3 = otherIn - GetPlayersCountByTeam(Team);
        else if (GetInvitedCount(Team) <= GetMinPlayersPerTeam())
            diff3 = GetMinPlayersPerTeam() - GetInvitedCount(Team) + 1;

        diff = std::min(diff, diff2);
        return std::min(diff, diff3);
    }
    return 0;
}

bool Battleground::HasFreeSlots() const
{
    return _players.size() < GetMaxPlayers();
}

void Battleground::BuildPvPLogDataPacket(WorldPackets::Battleground::PVPLogData& packet)
{
    uint8 bType = MS::Battlegrounds::GetBracketByJoinType(GetJoinType());

    if (GetStatus() == STATUS_WAIT_LEAVE)
        packet.Winner = m_Winner;

    packet.Players.reserve(GetPlayerScoresSize());
    for (auto const& score : PlayerScores)
    {
        if (!IsPlayerInBattleground(score.first))
            continue;

        WorldPackets::Battleground::PVPLogData::PlayerData playerData;

        playerData.PlayerGUID = score.second->PlayerGuid;
        playerData.Kills = score.second->KillingBlows;
        playerData.Faction = score.second->TeamID;
        if (score.second->HonorableKills || score.second->Deaths || score.second->BonusHonor)
        {
            playerData.Honor = boost::in_place();
            playerData.Honor->HonorKills = score.second->HonorableKills;
            playerData.Honor->Deaths = score.second->Deaths;
            playerData.Honor->ContributionPoints = score.second->BonusHonor;
        }

        playerData.DamageDone = score.second->DamageDone;
        playerData.HealingDone = score.second->HealingDone;
        score.second->BuildObjectivesBlock(playerData.Stats);

        if (Player* player = ObjectAccessor::FindPlayer(playerData.PlayerGUID))
        {
            auto const& bgPlayerData = _players[playerData.PlayerGUID];

            playerData.IsInWorld = true;
            playerData.PrimaryTalentTree = bgPlayerData.ActiveSpec;
            playerData.PrimaryTalentTreeNameIndex = bgPlayerData.PrimaryTalentTreeNameIndex;
            playerData.Race = player->getRace();
            playerData.PrestigeRank = bgPlayerData.PrestigeRank;

            if (IsRated())
            {
                Bracket* bracket = player->getBracket(bType);
                if (!bracket)
                    bracket = sBracketMgr->TryGetOrCreateBracket(score.first, bType);

                if (bracket)
                {
                    playerData.PreMatchRating = bracket->getRating();
                    playerData.RatingChange = bracket->getRatingLastChange();
                    playerData.PreMatchMMR = bracket->getMMV();
                    playerData.MmrChange = bracket->getMMV() - bracket->getLastMMRChange();
                }
            }
        }

        packet.Players.push_back(playerData);
    }

    if (IsRated())
    {
        packet.Ratings = boost::in_place();

        for (uint8 i = TEAM_ALLIANCE; i < MAX_TEAMS; ++i)
        {
            packet.Ratings->Postmatch[i] = _arenaTeamScores[i].NewRating;
            packet.Ratings->Prematch[i] = _arenaTeamScores[i].OldRating;
            packet.Ratings->PrematchMMR[i] = _arenaTeamScores[i].MatchmakerRating;
        }
    }

    packet.PlayerCount[0] = int8(GetPlayersCountByTeam(HORDE));
    packet.PlayerCount[1] = int8(GetPlayersCountByTeam(ALLIANCE));
}

bool Battleground::UpdatePlayerScore(Player* player, uint32 type, uint32 value, bool doAddHonor /*= true*/)
{
    std::map<ObjectGuid, BattlegroundScore*>::const_iterator itr = PlayerScores.find(player->GetGUID());
    if (itr == PlayerScores.end())
        return false;

    if (type == SCORE_BONUS_HONOR && doAddHonor && IsBattleground())
        player->RewardHonor(nullptr, 1, value);
    else
        itr->second->UpdateScore(type, value);

    return true;
}

uint32 Battleground::GetPlayersCountByTeam(uint32 Team) const
{
    return m_PlayersCount[MS::Battlegrounds::GetTeamIdByTeam(Team)];
}

uint32 Battleground::GetPlayerScoreByType(Player* player, uint32 type) const
{
    std::map<ObjectGuid, BattlegroundScore*>::const_iterator itr = PlayerScores.find(player->GetGUID());
    if (itr == PlayerScores.end())
        return 0;

    return itr->second->GetScore(type);
}

void Battleground::AddPlayerToResurrectQueue(ObjectGuid npc_guid, ObjectGuid playerGUID)
{
    m_ReviveQueue[npc_guid].push_back(playerGUID);
    if (Player* player = ObjectAccessor::FindPlayer(playerGUID))
        player->CastSpell(player, SPELL_WAITING_FOR_RESURRECT, true);
}

void Battleground::RemovePlayerFromResurrectQueue(ObjectGuid playerGUID)
{
    for (auto& itr : m_ReviveQueue)
    {
        for (auto itr2 = itr.second.begin(); itr2 != itr.second.end(); ++itr2)
        {
            if (*itr2 == playerGUID)
            {
                itr.second.erase(itr2);
                if (Player* player = ObjectAccessor::FindPlayer(playerGUID))
                    player->RemoveAurasDueToSpell(SPELL_WAITING_FOR_RESURRECT);
                return;
            }
        }
    }
}

bool Battleground::AddObject(uint32 type, uint32 entry, Position pos, Position rotation /*= { }*/, uint32 respawnTime /*= 0*/, GOState goState)
{
    return AddObject(type, entry, pos.m_positionX, pos.m_positionY, pos.m_positionZ, pos.m_orientation, rotation.m_positionX, rotation.m_positionY, rotation.m_positionZ, rotation.m_orientation, respawnTime, goState);
}

bool Battleground::AddObject(uint32 type, uint32 entry, float x, float y, float z, float o, float rotation0, float rotation1, float rotation2, float rotation3, uint32 /*respawnTime*/, GOState goState)
{
    ASSERT(type < BgObjects.size());

    Map* map = FindBgMap();
    if (!map)
        return false;

    GameObject* go = sObjectMgr->IsStaticTransport(entry) ? new StaticTransport : new GameObject;
    if (!go->Create(sObjectMgr->GetGenerator<HighGuid::GameObject>()->Generate(), entry, GetBgMap(), PHASEMASK_NORMAL, Position(x, y, z, o), G3D::Quat(rotation0, rotation1, rotation2, rotation3), 255, goState))
    {
        TC_LOG_ERROR(LOG_FILTER_BATTLEGROUND, "Battleground::AddObject: cannot create gameobject (entry: %u) for BG (map: %u, instance id: %u)!", entry, m_MapId, m_InstanceID);
        delete go;
        return false;
    }

    if (!map->AddToMap(go))
    {
        delete go;
        return false;
    }

    BgObjects[type] = go->GetGUID();
    return true;
}

void Battleground::DoorsOpen(uint32 type1, uint32 type2)
{
    DoorOpen(type1);
    DoorOpen(type2);
}

void Battleground::DoorsClose(uint32 type1, uint32 type2)
{
    DoorClose(type1);
    DoorClose(type2);
}

void Battleground::DoorClose(uint32 type)
{
    if (GameObject* obj = GetBgMap()->GetGameObject(BgObjects[type]))
    {
        if (obj->getLootState() == GO_ACTIVATED && obj->GetGoState() != GO_STATE_READY)
        {
            obj->SetLootState(GO_READY);
            obj->SetGoState(GO_STATE_READY);
        }
    }
    else
        TC_LOG_ERROR(LOG_FILTER_BATTLEGROUND, "Battleground::DoorClose: door gameobject (type: %u, GUID: %u) not found for BG (map: %u, instance id: %u)!", type, BgObjects[type].GetCounter(), m_MapId, m_InstanceID);
}

void Battleground::DoorOpen(uint32 type)
{
    if (GameObject* obj = GetBgMap()->GetGameObject(BgObjects[type]))
    {
        obj->SetLootState(GO_ACTIVATED);
        obj->SetGoState(obj->GetGoType() == GAMEOBJECT_TYPE_TRANSPORT ? GO_STATE_TRANSPORT_ACTIVE : GO_STATE_ACTIVE);
    }
    else
        TC_LOG_ERROR(LOG_FILTER_BATTLEGROUND, "Battleground::DoorOpen: door gameobject (type: %u, GUID: %u) not found for BG (map: %u, instance id: %u)!", type, BgObjects[type].GetCounter(), m_MapId, m_InstanceID);
}

GameObject* Battleground::GetBGObject(uint32 type)
{
    GameObject* obj = GetBgMap()->GetGameObject(BgObjects[type]);
    if (!obj)
        TC_LOG_ERROR(LOG_FILTER_BATTLEGROUND, "Battleground::GetBGObject: gameobject (type: %u, GUID: %u) not found for BG (map: %u, instance id: %u)!", type, BgObjects[type].GetCounter(), m_MapId, m_InstanceID);
    return obj;
}

Creature* Battleground::GetBGCreature(uint32 type)
{
    Creature* creature = GetBgMap()->GetCreature(BgCreatures[type]);
    if (!creature)
        TC_LOG_ERROR(LOG_FILTER_BATTLEGROUND, "Battleground::GetBGCreature: creature (type: %u, GUID: %u) not found for BG (map: %u, instance id: %u)!", type, BgCreatures[type].GetCounter(), m_MapId, m_InstanceID);
    return creature;
}

void Battleground::SpawnBGObject(uint32 type, uint32 respawntime)
{
    if (Map* map = FindBgMap())
        if (GameObject* obj = map->GetGameObject(BgObjects[type]))
        {
            if (respawntime)
                obj->SetLootState(GO_JUST_DEACTIVATED);
            else
                if (obj->getLootState() == GO_JUST_DEACTIVATED)
                    obj->SetLootState(GO_READY);
            obj->SetRespawnTime(respawntime);
            map->AddToMap(obj);
        }
}

Creature* Battleground::AddCreature(uint32 entry, uint32 type, uint32 teamval, Position pos, uint32 respawntime /*= 0*/, Transport* transport)
{
    return AddCreature(entry, type, teamval, pos.m_positionX, pos.m_positionY, pos.m_positionZ, pos.m_orientation, respawntime, transport);
}

Creature* Battleground::AddCreature(uint32 entry, uint32 type, uint32 teamval, float x, float y, float z, float o, uint32 respawntime, Transport* transport)
{
    ASSERT(type < BgCreatures.size());

    Map* map = FindBgMap();
    if (!map)
        return nullptr;

    if (transport)
    {
        if (Creature* creature = transport->SummonPassenger(entry, { x, y, z, o }, TEMPSUMMON_MANUAL_DESPAWN))
        {
            BgCreatures[type] = creature->GetGUID();
            return creature;
        }

        return nullptr;
    }

    auto creature = new Creature;
    if (!creature->Create(sObjectMgr->GetGenerator<HighGuid::Creature>()->Generate(), map, PHASEMASK_NORMAL, entry, 0, teamval, x, y, z, o))
    {
        TC_LOG_ERROR(LOG_FILTER_BATTLEGROUND, "Battleground::AddCreature: cannot create creature (entry: %u) for BG (map: %u, instance id: %u)!", entry, m_MapId, m_InstanceID);
        delete creature;
        return nullptr;
    }

    creature->SetHomePosition(x, y, z, o);

    CreatureTemplate const* cinfo = sObjectMgr->GetCreatureTemplate(entry);
    if (!cinfo)
    {
        TC_LOG_ERROR(LOG_FILTER_BATTLEGROUND, "Battleground::AddCreature: creature template (entry: %u) does not exist for BG (map: %u, instance id: %u)!", entry, m_MapId, m_InstanceID);
        delete creature;
        return nullptr;
    }

    creature->SetSpeed(MOVE_WALK, cinfo->speed_walk);
    creature->SetSpeed(MOVE_RUN, cinfo->speed_run);
    creature->SetSpeed(MOVE_FLIGHT, cinfo->speed_fly);

    if (!map->AddToMap(creature))
    {
        delete creature;
        return nullptr;
    }

    BgCreatures[type] = creature->GetGUID();

    if (respawntime)
        creature->SetRespawnDelay(respawntime);

    return  creature;
}

bool Battleground::DelCreature(uint32 type)
{
    if (!BgCreatures[type])
        return true;

    if (Creature* creature = GetBgMap()->GetCreature(BgCreatures[type]))
    {
        creature->AddObjectToRemoveList();
        BgCreatures[type].Clear();
        return true;
    }

    TC_LOG_ERROR(LOG_FILTER_BATTLEGROUND, "Battleground::DelCreature: creature (type: %u, GUID: %u) not found for BG (map: %u, instance id: %u)!", type, BgCreatures[type].GetCounter(), m_MapId, m_InstanceID);
    BgCreatures[type].Clear();
    return false;
}

bool Battleground::DelObject(uint32 type)
{
    if (!BgObjects[type])
        return true;

    if (GameObject* obj = GetBgMap()->GetGameObject(BgObjects[type]))
    {
        obj->SetRespawnTime(0);
        obj->Delete();
        BgObjects[type].Clear();
        return true;
    }

    TC_LOG_ERROR(LOG_FILTER_BATTLEGROUND, "Battleground::DelObject: gameobject (type: %u, GUID: %u) not found for BG (map: %u, instance id: %u)!", type, BgObjects[type].GetCounter(), m_MapId, m_InstanceID);
    BgObjects[type].Clear();
    return false;
}

bool Battleground::AddSpiritGuide(uint32 type, DBCPosition4D loc, TeamId team)
{
    return AddSpiritGuide(type, loc.X, loc.Y, loc.Z, loc.O, MS::Battlegrounds::GetTeamByTeamId(team));
}

bool Battleground::AddSpiritGuide(uint32 type, Position pos, TeamId team)
{
    return AddSpiritGuide(type, pos.m_positionX, pos.m_positionY, pos.m_positionZ, pos.m_orientation, MS::Battlegrounds::GetTeamByTeamId(team));
}

bool Battleground::AddSpiritGuide(uint32 type, float x, float y, float z, float o, uint32 team)
{
    uint32 entry = team == ALLIANCE ? BG_CREATURE_ENTRY_A_SPIRITGUIDE : BG_CREATURE_ENTRY_H_SPIRITGUIDE;

    if (Creature* creature = AddCreature(entry, type, team, x, y, z, o))
    {
        creature->setDeathState(DEAD);
        creature->AddChannelObject(creature->GetGUID());
        creature->SetChannelSpellId(SPELL_BG_SPIRIT_HEAL_CHANNEL);
        creature->SetChannelSpellXSpellVisualId(3060);
        creature->SetFloatValue(UNIT_FIELD_MOD_CASTING_SPEED, 1.0f);
        creature->SetFloatValue(UNIT_FIELD_MOD_SPELL_HASTE, 1.0f);
        return true;
    }

    TC_LOG_ERROR(LOG_FILTER_BATTLEGROUND, "Battleground::AddSpiritGuide: cannot create spirit guide (type: %u, entry: %u) for BG (map: %u, instance id: %u)!", type, entry, m_MapId, m_InstanceID);
    EndNow();
    return false;
}

void Battleground::PSendMessageToAll(int32 entry, ChatMsg msgType, Player const* source, ...)
{
    if (!entry)
        return;

    va_list ap;
    va_start(ap, source);

    Trinity::TrinityStringChatBuilder builder(nullptr, msgType, entry, source, &ap);
    Trinity::LocalizedPacketDo<Trinity::TrinityStringChatBuilder> localizer(builder);
    BroadcastWorker(localizer);

    va_end(ap);
}

void Battleground::SendBroadcastText(int32 broadcastTextID, ChatMsg type, WorldObject const* target /*= nullptr*/)
{
    if (!sBroadcastTextStore.LookupEntry(broadcastTextID))
        return;

    Trinity::BroadcastTextBuilder builder(nullptr, type, broadcastTextID, target);
    Trinity::LocalizedPacketDo<Trinity::BroadcastTextBuilder> localizer(builder);
    BroadcastWorker(localizer);
}

void Battleground::EndNow()
{
    RemoveFromBGFreeSlotQueue();
    SetStatus(STATUS_WAIT_LEAVE);
    m_EndTime = Milliseconds(0);
}

bool Battleground::HandlePlayerUnderMap(Player* player)
{
    if (Battleground* bg = player->GetBattleground())
        bg->EventPlayerDroppedFlag(player);

    player->TeleportTo(GetMapId(), GetTeamStartPosition(player->GetBGTeamId()), TELE_TO_NOT_LEAVE_COMBAT);
    return true;
}

// To be removed
const char* Battleground::GetTrinityString(int32 entry)
{
    // FIXME: now we have different DBC locales and need localized message for each target client
    return sObjectMgr->GetTrinityStringForDBCLocale(entry);
}

void Battleground::HandleTriggerBuff(ObjectGuid go_guid)
{
    if (!GetBgMap())
        return;
    GameObject* obj = GetBgMap()->GetGameObject(go_guid);
    if (!obj || obj->GetGoType() != GAMEOBJECT_TYPE_TRAP || !obj->isSpawned())
        return;

    // Change buff type, when buff is used:
    int32 index = BgObjects.size() - 1;
    while (index >= 0 && BgObjects[index] != go_guid)
        index--;
    if (index < 0)
    {
        TC_LOG_ERROR(LOG_FILTER_BATTLEGROUND, "Battleground::HandleTriggerBuff: cannot find buff gameobject (GUID: %u, entry: %u, type: %u) in internal data for BG (map: %u, instance id: %u)!", go_guid.GetCounter(), obj->GetEntry(), obj->GetGoType(), m_MapId, m_InstanceID);
        return;
    }

    uint8 buff = urand(0, 2);
    uint32 entry = obj->GetEntry();
    if (m_BuffChange && entry != Buff_Entries[buff])
    {
        SpawnBGObject(index, RESPAWN_ONE_DAY);
        for (uint8 currBuffTypeIndex = 0; currBuffTypeIndex < 3; ++currBuffTypeIndex)
            if (entry == Buff_Entries[currBuffTypeIndex])
            {
                index -= currBuffTypeIndex;
                index += buff;
            }
    }

    SpawnBGObject(index, (IsBrawl() ? 0.5f: 1) * BUFF_RESPAWN_TIME);
}

void Battleground::HandleStartTimer(TimerType type)
{
    if (type != WORLD_TIMER_TYPE_PVP)
        return;

    Seconds countdownMaxForBGType = IsArena() ? CalcArenaCountdown(GetJoinType()) : Minutes(2);

    if (GetElapsedTime() >= countdownMaxForBGType)
        return;

    SetCountdownTimer(Seconds(0));

    WorldPackets::Instance::StartTimer startTimer;
    startTimer.Type = type;
    startTimer.TimeRemaining = countdownMaxForBGType - std::chrono::duration_cast<Seconds>(GetElapsedTime());
    startTimer.TotalTime = countdownMaxForBGType;
    SendPacketToAll(startTimer.Write());
}

void Battleground::HandleKillPlayer(Player* victim, Player* killer)
{
    UpdatePlayerScore(victim, SCORE_DEATHS, 1, !IsWargame());
    if (killer)
    {
        if (killer == victim)
            return;

        UpdatePlayerScore(killer, SCORE_HONORABLE_KILLS, 1, !IsWargame());
        UpdatePlayerScore(killer, SCORE_KILLING_BLOWS, 1, !IsWargame());

        for (auto const& itr : GetPlayers())
        {
            Player* creditedPlayer = ObjectAccessor::FindPlayer(itr.first);
            if (!creditedPlayer || creditedPlayer == killer)
                continue;

            if (creditedPlayer->GetBGTeam() == killer->GetBGTeam() && creditedPlayer->IsAtGroupRewardDistance(victim))
                UpdatePlayerScore(creditedPlayer, SCORE_HONORABLE_KILLS, 1, !IsWargame());
        }
    }

    if (!IsArena() && !IsWargame())
    {
        victim->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE);
        RewardXPAtKill(killer, victim);
    }
}

uint32 Battleground::GetPlayerTeam(ObjectGuid guid) const
{
    auto itr = _players.find(guid);
    if (itr != _players.end())
        return itr->second.Team;
    return 0;
}

uint32 Battleground::GetOtherTeam(uint32 teamId) const
{
    return teamId ? (teamId == ALLIANCE ? HORDE : ALLIANCE) : 0;
}

bool Battleground::IsPlayerInBattleground(ObjectGuid guid) const
{
    return _players.find(guid) != _players.end();
}

void Battleground::PlayerAddedToBGCheckIfBGIsRunning(Player* player)
{
    if (GetStatus() != STATUS_WAIT_LEAVE)
        return;

    BlockMovement(player);

    WorldPackets::Battleground::PVPLogData pvpLogData;
    BuildPvPLogDataPacket(pvpLogData);
    player->SendDirectMessage(pvpLogData.Write());

    auto queueTypeID = MS::Battlegrounds::GetBgQueueTypeIdByBgTypeID(GetTypeID(), GetJoinType());

    WorldPackets::Battleground::BattlefieldStatusActive battlefieldStatus;
    sBattlegroundMgr->BuildBattlegroundStatusActive(&battlefieldStatus, this, player, player->GetBattlegroundQueueIndex(queueTypeID), player->GetBattlegroundQueueJoinTime(queueTypeID), GetJoinType());
    player->SendDirectMessage(battlefieldStatus.Write());
}

uint32 Battleground::GetAlivePlayersCountByTeam(uint32 Team) const
{
    uint32 count = 0;
    for (auto const& itr : GetPlayers())
        if (itr.second.Team == Team)
            if (Player* player = ObjectAccessor::FindPlayer(itr.first))
                if (player->isAlive() && player->GetShapeshiftForm() != FORM_SPIRITOFREDEMPTION && (player->InArena() || (IsBrawl() && player->InBattleground())))
                    ++count;

    return count;
}

void Battleground::UpdatePlayersCountByTeam(uint32 Team, bool remove)
{
    if (remove)
        --m_PlayersCount[MS::Battlegrounds::GetTeamIdByTeam(Team)];
    else
        ++m_PlayersCount[MS::Battlegrounds::GetTeamIdByTeam(Team)];
}

int32 Battleground::GetObjectType(ObjectGuid guid)
{
    for (auto i = 0; i < BgObjects.size(); ++i)
        if (BgObjects[i] == guid)
            return i;

    TC_LOG_ERROR(LOG_FILTER_BATTLEGROUND, "Battleground::GetObjectType: player used gameobject (GUID: %u) which is not in internal data for BG (map: %u, instance id: %u), cheating?", guid.GetCounter(), m_MapId, m_InstanceID);
    return -1;
}

void Battleground::BattlegroundTimedWin(uint32 type /*= 1*/)
{
    switch (type)
    {
        case 1:
            if (m_TeamScores[TEAM_ALLIANCE] > m_TeamScores[TEAM_HORDE])
                EndBattleground(ALLIANCE);
            else if (m_TeamScores[TEAM_HORDE] > m_TeamScores[TEAM_ALLIANCE])
                EndBattleground(HORDE);
            else
                EndBattleground(TEAM_OTHER);
            break;
        case 2:
            if (m_TeamScores[TEAM_ALLIANCE] == 0)
            {
                if (m_TeamScores[TEAM_HORDE] == 0)
                    EndBattleground(WINNER_NONE);
                else
                    EndBattleground(HORDE);
            }
            else if (m_TeamScores[TEAM_HORDE] == 0)
                EndBattleground(ALLIANCE);
            else if (m_TeamScores[TEAM_HORDE] == m_TeamScores[TEAM_ALLIANCE])
                EndBattleground(m_lastFlagCaptureTeam);
            else if (m_TeamScores[TEAM_HORDE] > m_TeamScores[TEAM_ALLIANCE])
                EndBattleground(HORDE);
            else
                EndBattleground(ALLIANCE);
            break;
        default:
            break;
    }
}

void Battleground::SetBgRaid(uint32 TeamID, Group* bg_raid)
{
    Group*& old_raid = TeamID == ALLIANCE ? m_BgRaids[TEAM_ALLIANCE] : m_BgRaids[TEAM_HORDE];
    if (old_raid)
        old_raid->SetBattlegroundGroup(nullptr);
    if (bg_raid)
        bg_raid->SetBattlegroundGroup(this);
    old_raid = bg_raid;
}

WorldSafeLocsEntry const* Battleground::GetClosestGraveYard(Player* player)
{
    return sObjectMgr->GetClosestGraveYard(player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), player->GetMapId(), player->GetBGTeam());
}

bool Battleground::IsTeamScoreInRange(uint32 team, uint32 minScore, uint32 maxScore) const
{
    uint32 score = std::max(m_TeamScores[MS::Battlegrounds::GetTeamIdByTeam(team)], 0);
    return score >= minScore && score <= maxScore;
}

void Battleground::StartTimedAchievement(CriteriaTimedTypes type, uint32 entry)
{
    for (auto const& itr : GetPlayers())
        if (Player* player = ObjectAccessor::FindPlayer(itr.first))
            player->GetAchievementMgr()->StartTimedAchievement(type, entry);
}

Seconds Battleground::CalcArenaCountdown(uint8 joinType) const
{
    switch (joinType)
    {
        case MS::Battlegrounds::JoinType::Arena1v1:
        {
            uint32 timer = sWorld->getIntConfig(CONFIG_ARENA_1V1_COUNTDOWN);
            return Seconds(timer ? timer : ARENA_1V1_COUNTDOWN);
        }
        case MS::Battlegrounds::JoinType::Arena2v2:
        case MS::Battlegrounds::JoinType::Skirmish2v2:
        {
            uint32 timer = sWorld->getIntConfig(CONFIG_ARENA_2V2_COUNTDOWN);
            return Seconds(timer ? timer : ARENA_2V2_COUNTDOWN);
        }
        default:
        {
            uint32 timer = sWorld->getIntConfig(CONFIG_ARENA_3V3_COUNTDOWN);
            return Seconds(timer ? timer : ARENA_COUNTDOWN_MAX);
        }
    }
}

uint16 Battleground::GetTypeID(bool GetRandom) const
{
    return GetRandom ? m_RandomTypeID : m_TypeID;
}

void Battleground::SetBracket(PVPDifficultyEntry const* bracketEntry)
{
    m_BracketId = bracketEntry->RangeIndex;
    SetLevelRange(bracketEntry->MinLevel, bracketEntry->MaxLevel);
}

void Battleground::RewardXPAtKill(Player* killer, Player* victim)
{
    if (IsWargame())
        return;

    if (sWorld->getBoolConfig(CONFIG_BG_XP_FOR_KILL) && killer && victim)
        killer->RewardPlayerAndGroupAtKill(victim, true);
}

void Battleground::_CheckPositions(uint32 diff)
{
    if (GetStatus() != STATUS_WAIT_JOIN)
        return;

    m_ValidStartPositionTimer += diff;
    if (m_ValidStartPositionTimer < 5000)
        return;

    m_ValidStartPositionTimer = 0;

    switch (GetMapId())
    {
        case 607: // Strand of the Ancients 
        case 1101: // Deathmatch
        case 1803: // seething shore
            return;
        default:
            break;
    }

    for (auto const& itr : GetPlayers())
        if (auto player = ObjectAccessor::FindPlayer(itr.first))
        {
            auto const* startPos = GetTeamStartPosition(player->GetBGTeamId());
            if (player->GetExactDist2d(startPos) > 70.0f)
                player->TeleportTo(GetMapId(), startPos);
        }
}

void Battleground::_ProcessPlayerPositionBroadcast(Milliseconds diff)
{
    if (m_LastPlayerPositionBroadcast > diff)
    {
        m_LastPlayerPositionBroadcast -= diff;
        return;
    }

    m_LastPlayerPositionBroadcast = PositionBroadcastUpdate;

    WorldPackets::Battleground::PlayerPositions playerPositions;
    if (!IsArena())
        GetPlayerPositionData(&playerPositions.FlagCarriers);
    SendPacketToAll(playerPositions.Write());
}

void Battleground::SendSpectateAddonsMsg(SpectatorAddonMsg& msg)
{
    if (HaveSpectators())
        for (auto v : _spectators)
            msg.SendPacket(v);
}

void Battleground::AddSpectator(Player* player)
{
    if (!player)
        return;

    _spectators.insert(player->GetGUID());
    player->SetSpectateRemoving(false);
}

void Battleground::RemoveSpectator(Player* player)
{
    _spectators.erase(player->GetGUID());
    player->SetSpectateRemoving(true);
}
