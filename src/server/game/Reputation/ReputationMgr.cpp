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

#include "DatabaseEnv.h"
#include "ReputationMgr.h"
#include "Player.h"
#include "WorldPacket.h"
#include "World.h"
#include "ObjectMgr.h"
#include "ScriptMgr.h"
#include "ReputationPackets.h"
#include "CharacterPackets.h"
#include "QuestData.h"

ReputationRank ReputationMgr::GetReactionLevel(int32 standing)
{
    // copied from client FactionRec::GetReactionLevel
    ReputationRank rank = REP_EXALTED;
    if (standing < 42000)
    {
        rank = REP_REVERED;
        if (standing < 21000)
        {
            rank = REP_HONORED;
            if (standing < 9000)
            {
                rank = REP_FRIENDLY;
                if (standing < 3000)
                {
                    rank = REP_NEUTRAL;
                    if (standing < 0)
                    {
                        rank = REP_UNFRIENDLY;
                        if (standing < -3000)
                            rank = (standing >= -6000) ? REP_HOSTILE : REP_HATED;
                    }
                }
            }
        }
    }

    return rank;
}

uint8 ReputationMgr::GetVisibleFactionCount() const
{
    return _visibleFactionCount;
}

uint8 ReputationMgr::GetHonoredFactionCount() const
{
    return _honoredFactionCount;
}

uint8 ReputationMgr::GetReveredFactionCount() const
{
    return _reveredFactionCount;
}

uint8 ReputationMgr::GetExaltedFactionCount() const
{
    return _exaltedFactionCount;
}

FactionStateList const& ReputationMgr::GetStateList() const
{
    return _factions;
}

FactionState const* ReputationMgr::GetState(FactionEntry const* factionEntry) const
{
    return factionEntry->CanHaveReputation() ? GetState(factionEntry->ReputationIndex) : nullptr;
}

FactionState const* ReputationMgr::GetState(RepListID id) const
{
    if (_factions.empty())
        return nullptr;

    if (id >= 0 && id < WorldPackets::Reputation::FactionCount)
    {
        if (_factions[id].init)
            return &_factions[id];
    }

    return nullptr;
}

FactionState* ReputationMgr::GetState(RepListID id)
{
    if (_factions.empty())
        return nullptr;

    if (id >= 0 && id < WorldPackets::Reputation::FactionCount)
    {
        if (_factions[id].init)
            return &_factions[id];
    }

    return nullptr;
}

bool ReputationMgr::IsAtWar(uint32 faction_id) const
{
    FactionEntry const* factionEntry = sFactionStore.LookupEntry(faction_id);

    if (!factionEntry)
    {
        TC_LOG_DEBUG(LOG_FILTER_GENERAL, "ReputationMgr::IsAtWar: Can't get AtWar flag of %s for unknown faction (faction id) #%u.", _player->GetName(), faction_id);
        return false;
    }

    return IsAtWar(factionEntry);
}

bool ReputationMgr::IsAtWar(FactionEntry const* factionEntry) const
{
    if (!factionEntry)
        return false;

    if (FactionState const* factionState = GetState(factionEntry))
        return (factionState->Flags & FACTION_FLAG_AT_WAR) != 0;
    return false;
}

int32 ReputationMgr::GetReputation(uint32 faction_id) const
{
    FactionEntry const* factionEntry = sFactionStore.LookupEntry(faction_id);

    if (!factionEntry)
    {
        TC_LOG_DEBUG(LOG_FILTER_GENERAL, "ReputationMgr::GetReputation: Can't get reputation of %s for unknown faction (faction id) #%u.", _player->GetName(), faction_id);
        return 0;
    }

    return GetReputation(factionEntry);
}

int32 ReputationMgr::GetBaseReputation(FactionEntry const* factionEntry) const
{
    if (!factionEntry)
        return 0;

    uint64 raceMask = _player->getRaceMask();
    uint32 classMask = _player->getClassMask();
    for (uint8 i = 0; i < 4; ++i)
    {
        if ((factionEntry->ReputationRaceMask[i] & raceMask || (factionEntry->ReputationRaceMask[i] == 0 && factionEntry->ReputationClassMask[i] != 0)) &&
            (factionEntry->ReputationClassMask[i] & classMask || factionEntry->ReputationClassMask[i] == 0))
        {
            return factionEntry->ReputationBase[i];
        }
    }

    // in faction.dbc exist factions with (RepListId >=0, listed in character reputation list) with all ReputationRaceMask[i] == 0
    return 0;
}

int32 ReputationMgr::GetReputation(FactionEntry const* factionEntry) const
{
    // Faction without recorded reputation. Just ignore.
    if (!factionEntry)
        return 0;

    if (FactionState const* state = GetState(factionEntry))
        return GetBaseReputation(factionEntry) + state->Standing;

    return 0;
}

ReputationRank ReputationMgr::GetRank(FactionEntry const* factionEntry) const
{
    int32 reputation = GetReputation(factionEntry);
    return GetReactionLevel(reputation);
}

ReputationRank ReputationMgr::GetBaseRank(FactionEntry const* factionEntry) const
{
    int32 reputation = GetBaseReputation(factionEntry);
    return GetReactionLevel(reputation);
}

uint32 ReputationMgr::GetReputationRankStrIndex(FactionEntry const* factionEntry) const
{
    return ReputationRankStrIndex[GetRank(factionEntry)];
}

ReputationRank const* ReputationMgr::GetForcedRankIfAny(uint32 factionId) const
{
    return Trinity::Containers::MapGetValuePtr(_forcedReactions, factionId);
}

ReputationRank const* ReputationMgr::GetForcedRankIfAny(FactionTemplateEntry const* factionTemplateEntry) const
{
    return GetForcedRankIfAny(factionTemplateEntry->Faction);
}

bool ReputationMgr::SetReputation(FactionEntry const* factionEntry, int32 standing)
{
    return SetReputation(factionEntry, standing, false, false);
}

bool ReputationMgr::ModifyReputation(FactionEntry const* factionEntry, int32 standing, bool noSpillover)
{
    return SetReputation(factionEntry, standing, true, noSpillover);
}

void ReputationMgr::ModifyParagonReputation(FactionEntry const* factionEntry, int32 standing)
{
    if (!sWorld->getBoolConfig(CONFIG_PARAGON_ENABLE))
        return;

    if (!factionEntry || !factionEntry->ParagonFactionID)
        return;

    FactionState* factionParent = GetState(factionEntry->ReputationIndex);
    if (!factionParent)
        return;

    if ((factionParent->Standing + GetBaseReputation(factionEntry)) < Reputation_Cap)
        return;

    FactionEntry const* factionParagon = sFactionStore.LookupEntry(factionEntry->ParagonFactionID);
    ParagonReputationEntry const* paragonReputation = sDB2Manager.GetFactionParagon(factionEntry->ParagonFactionID);
    if (factionParagon && paragonReputation)
    {
        if (auto faction = GetState(factionParagon->ReputationIndex))
        {
            SetOneFactionReputation(factionParagon, standing, true, true);
            faction->Flags = FACTION_FLAG_RIVAL | FACTION_FLAG_HIDDEN;
            if (faction->Standing >= paragonReputation->LevelThreshold && _player->GetQuestStatus(paragonReputation->QuestID) == QUEST_STATUS_NONE)
            {
                faction->Standing -= paragonReputation->LevelThreshold;
                if (Quest const* quest = sQuestDataStore->GetQuestTemplate(paragonReputation->QuestID))
                    if (_player->CanTakeQuest(quest, false))
                        _player->AddQuest(quest, _player);
            }

            _player->AddDelayedEvent(200, [=]() -> void
            {
                SendState(faction);
            });
        }
    }
}

void ReputationMgr::ApplyForceReaction(uint32 faction_id, ReputationRank rank, bool apply)
{
    if (apply)
        _forcedReactions[faction_id] = rank;
    else
        _forcedReactions.erase(faction_id);
}

uint32 ReputationMgr::GetDefaultStateFlags(FactionEntry const* factionEntry) const
{
    if (!factionEntry)
        return 0;

    uint64 raceMask = _player->getRaceMask();
    uint16 classMask = _player->getClassMask();

    for (uint8 i = 0; i < 4; i++)
        if ((factionEntry->ReputationRaceMask[i] & raceMask  || (factionEntry->ReputationRaceMask[i] == 0  &&
             factionEntry->ReputationClassMask[i] != 0)) && (factionEntry->ReputationClassMask[i] & classMask || factionEntry->ReputationClassMask[i] == 0))
            return factionEntry->ReputationFlags[i];

    return 0;
}

void ReputationMgr::SendForceReactions()
{
    WorldPackets::Reputation::SetForcedReactions reactions;
    reactions.Reactions.resize(_forcedReactions.size());

    std::size_t i = 0;
    for (const auto& itr : _forcedReactions)
    {
        WorldPackets::Reputation::ForcedReaction& fReaction = reactions.Reactions[i++];
        fReaction.Faction = itr.first;
        fReaction.Reaction = uint32(itr.second);
    }

    _player->SendDirectMessage(reactions.Write());
}

void ReputationMgr::SendState(FactionState const* faction)
{
    WorldPackets::Reputation::SetFactionStanding standing;
    standing.BonusFromAchievementSystem = 0.0f;
    standing.ReferAFriendBonus = 0.0f;
    standing.ShowVisual = _sendFactionIncreased;

    WorldPackets::Reputation::FactionStandingData s1;
    s1.Index = faction->ReputationListID;
    s1.Standing = faction->Standing;
    standing.Faction.push_back(s1);

    for (uint16 i = 0; i < WorldPackets::Reputation::FactionCount; ++i)
    {
        FactionState* factionState = GetState(i);
        if (!factionState || !factionState->needSend)
            continue;

        factionState->needSend = false;
        if (factionState->ReputationListID != faction->ReputationListID)
        {
            WorldPackets::Reputation::FactionStandingData s;
            s.Index = factionState->ReputationListID;
            s.Standing = factionState->Standing;
            standing.Faction.push_back(s);
        }
    }
    
    _player->SendDirectMessage(standing.Write());

    _sendFactionIncreased = false; // Reset
}

void ReputationMgr::SendInitialReputations()
{
    WorldPackets::Reputation::InitializeFactions initFactions;

    for (uint16 i = 0; i < WorldPackets::Reputation::FactionCount; ++i)
    {
        FactionState* faction = GetState(i);
        if (!faction)
            continue;

        initFactions.FactionFlags[i] = faction->Flags;
        initFactions.FactionStandings[i] = faction->Standing;
        faction->needSend = false;
    }

    _player->SendDirectMessage(initFactions.Write());
}

void ReputationMgr::SendStates()
{
    for (uint16 i = 0; i < WorldPackets::Reputation::FactionCount; ++i)
        if (FactionState* faction = GetState(i))
            SendState(faction);
}

void ReputationMgr::SendVisible(FactionState const* faction, bool visible /*= true*/) const
{
    if (_player->GetSession()->PlayerLoading())
        return;

    WorldPackets::Character::SetFactionVisible packet(visible);
    packet.FactionIndex = faction->ReputationListID;
    _player->SendDirectMessage(packet.Write());
}

void ReputationMgr::Initialize()
{
    _factions.clear();
    _factions.resize(WorldPackets::Reputation::FactionCount);

    _visibleFactionCount = 0;
    _honoredFactionCount = 0;
    _reveredFactionCount = 0;
    _exaltedFactionCount = 0;
    _sendFactionIncreased = false;

    for (FactionEntry const* factionEntry : sFactionStore)
    {
        if (factionEntry->CanHaveReputation())
        {
            FactionState& newFaction = _factions[factionEntry->ReputationIndex];
            newFaction.ID = factionEntry->ID;
            newFaction.ReputationListID = factionEntry->ReputationIndex;
            newFaction.Standing = 0;
            newFaction.Flags = GetDefaultStateFlags(factionEntry);
            newFaction.needSend = false;
            newFaction.needSave = false;
            newFaction.init = true;
            newFaction.notInDB = true;
            newFaction.needUpdateStanding = false;
            newFaction.needUpdateFlags = false;

            if (newFaction.Flags & FACTION_FLAG_VISIBLE)
                ++_visibleFactionCount;

            if (!(newFaction.Flags & FACTION_FLAG_HIDDEN))
                UpdateRankCounters(REP_HOSTILE, GetBaseRank(factionEntry));
        }
    }
}

bool ReputationMgr::SetReputation(FactionEntry const* factionEntry, int32 standing, bool incremental, bool noSpillover)
{
    sScriptMgr->OnPlayerReputationChange(_player, factionEntry->ID, standing, incremental);
    bool res = false;
    if (!noSpillover)
    {
        if (auto repTemplate = sObjectMgr->GetRepSpilloverTemplate(factionEntry->ID))
        {
            for (uint32 i = 0; i < MAX_SPILLOVER_FACTIONS; ++i)
                if (repTemplate->faction[i])
                    if (_player->GetReputationRank(repTemplate->faction[i]) <= ReputationRank(repTemplate->faction_rank[i]))
                        SetOneFactionReputation(sFactionStore.AssertEntry(repTemplate->faction[i]), int32(standing * repTemplate->faction_rate[i]), incremental, false);
        }
        else
        {
            float spillOverRepOut = float(standing);
            auto flist = sDB2Manager.GetFactionTeamList(factionEntry->ID);
            if (!flist && factionEntry->ParentFactionID && factionEntry->ParentFactionMod[1] != 0.0f)
            {
                spillOverRepOut *= factionEntry->ParentFactionMod[1];
                if (FactionEntry const* parent = sFactionStore.LookupEntry(factionEntry->ParentFactionID))
                {
                    FactionState const* parentState = GetState(parent->ReputationIndex);
                    if (parentState && parentState->Flags & FACTION_FLAG_SPECIAL)
                        SetOneFactionReputation(parent, int32(spillOverRepOut), incremental, false);
                    else
                        flist = sDB2Manager.GetFactionTeamList(factionEntry->ParentFactionID);
                }
            }
            if (flist)
            {
                for (std::vector<uint32>::const_iterator itr = flist->begin(); itr != flist->end(); ++itr)
                {
                    if (FactionEntry const* factionEntryCalc = sFactionStore.LookupEntry(*itr))
                    {
                        if (factionEntryCalc == factionEntry || GetRank(factionEntryCalc) > ReputationRank(factionEntryCalc->ParentFactionCap[0]))
                            continue;

                        int32 spilloverRep = int32(spillOverRepOut * factionEntryCalc->ParentFactionMod[0]);
                        if (spilloverRep != 0 || !incremental)
                            res = SetOneFactionReputation(factionEntryCalc, spilloverRep, incremental, false);
                    }
                }
            }
        }
    }

    if (auto faction = GetState(factionEntry->ReputationIndex))
    {
        res = SetOneFactionReputation(factionEntry, standing, incremental, false);
        SendState(faction);
    }

    return res;
}

bool ReputationMgr::SetOneFactionReputation(FactionEntry const* factionEntry, int32 standing, bool incremental, bool paragon)
{
    if (FactionState* faction = GetState(factionEntry->ReputationIndex))
    {
        int32 BaseRep = GetBaseReputation(factionEntry);
        //if (BaseRep == Reputation_Bottom)
        //    return false;

        if (incremental)
        {
            // int32 *= float cause one point loss?
            if (!paragon)
                standing = int32(floor((float)standing * sWorld->getRate(RATE_REPUTATION_GAIN) + 0.5f));
            standing += faction->Standing + BaseRep;
        }

        if (standing > Reputation_Cap)
            standing = Reputation_Cap;
        else if (standing < Reputation_Bottom)
            standing = Reputation_Bottom;

        ReputationRank old_rank = GetReactionLevel(faction->Standing + BaseRep);
        ReputationRank new_rank = GetReactionLevel(standing);

        faction->Standing = standing - BaseRep;
        faction->needSend = true;
        if (faction->notInDB)
            faction->needSave = true;
        else
            faction->needUpdateStanding = true;

        if (!sDB2Manager.GetFactionParagon(factionEntry->ID))
            SetVisible(faction);

        if (new_rank <= REP_HOSTILE)
            SetAtWar(faction, true);

        if (new_rank > old_rank)
            _sendFactionIncreased = true;

        if (!(faction->Flags & FACTION_FLAG_HIDDEN))
            UpdateRankCounters(old_rank, new_rank);

        _player->ReputationChanged(factionEntry);
        _player->UpdateAchievementCriteria(CRITERIA_TYPE_KNOWN_FACTIONS,          factionEntry->ID);
        _player->UpdateAchievementCriteria(CRITERIA_TYPE_GAIN_REPUTATION,         factionEntry->ID);
        _player->UpdateAchievementCriteria(CRITERIA_TYPE_GAIN_EXALTED_REPUTATION, factionEntry->ID);
        _player->UpdateAchievementCriteria(CRITERIA_TYPE_GAIN_REVERED_REPUTATION, factionEntry->ID);
        _player->UpdateAchievementCriteria(CRITERIA_TYPE_GAIN_HONORED_REPUTATION, factionEntry->ID);

        return true;
    }

    return false;
}

void ReputationMgr::SetVisible(FactionTemplateEntry const*factionTemplateEntry)
{
    if (!factionTemplateEntry->Faction)
        return;

    if (FactionEntry const* factionEntry = sFactionStore.LookupEntry(factionTemplateEntry->Faction))
        // Never show factions of the opposing team
        if (!(factionEntry->ReputationRaceMask[1] & _player->getRaceMask() && factionEntry->ReputationBase[1] == Reputation_Bottom))
            SetVisible(factionEntry);
}

void ReputationMgr::SetVisible(FactionEntry const* factionEntry)
{
    if (!factionEntry->CanHaveReputation())
        return;

    if (FactionState* faction = GetState(factionEntry->ReputationIndex))
        SetVisible(faction);
}

void ReputationMgr::SetVisible(FactionState* faction)
{
    // always invisible or hidden faction can't be make visible
    // except if faction has FACTION_FLAG_SPECIAL
    if (faction->Flags & (FACTION_FLAG_INVISIBLE_FORCED|FACTION_FLAG_HIDDEN) && !(faction->Flags & FACTION_FLAG_SPECIAL))
        return;

    // already set
    if (faction->Flags & FACTION_FLAG_VISIBLE)
        return;

    faction->Flags |= FACTION_FLAG_VISIBLE;
    faction->needSend = true;
    faction->needUpdateFlags = true;

    ++_visibleFactionCount;

    SendVisible(faction);
}

bool ReputationMgr::IsVisible(RepListID repListID) const
{
    if (FactionState const* faction = GetState(repListID))
        return faction->Flags & FACTION_FLAG_VISIBLE;

    return false;
}

void ReputationMgr::SetAtWar(RepListID repListID, bool on)
{
    FactionState* faction = GetState(repListID);
    if (!faction)
        return;

    // always invisible or hidden faction can't change war state
    if (faction->Flags & (FACTION_FLAG_INVISIBLE_FORCED|FACTION_FLAG_HIDDEN))
        return;

    SetAtWar(faction, on);
}

void ReputationMgr::SetAtWar(FactionState* faction, bool atWar) const
{
    // not allow declare war to own faction.
    if (atWar && (faction->Flags & FACTION_FLAG_PEACE_FORCED))
        return;

    // already set
    if (((faction->Flags & FACTION_FLAG_AT_WAR) != 0) == atWar)
        return;

    if (atWar)
        faction->Flags |= FACTION_FLAG_AT_WAR;
    else
        faction->Flags &= ~FACTION_FLAG_AT_WAR;

    faction->needSend = true;
    faction->needUpdateFlags = true;
}

void ReputationMgr::SetInactive(RepListID repListID, bool on)
{
    if (FactionState* faction = GetState(repListID))
        SetInactive(faction, on);
}

void ReputationMgr::SetInactive(FactionState* faction, bool inactive) const
{
    // always invisible or hidden faction can't be inactive
    if (inactive && ((faction->Flags & (FACTION_FLAG_INVISIBLE_FORCED|FACTION_FLAG_HIDDEN)) || !(faction->Flags & FACTION_FLAG_VISIBLE)))
        return;

    // already set
    if (((faction->Flags & FACTION_FLAG_INACTIVE) != 0) == inactive)
        return;

    if (inactive)
        faction->Flags |= FACTION_FLAG_INACTIVE;
    else
        faction->Flags &= ~FACTION_FLAG_INACTIVE;

    faction->needSend = true;
    faction->needUpdateFlags = true;
}

void ReputationMgr::LoadFromDB(PreparedQueryResult result)
{
    // Set initial reputations (so everything is nifty before DB data load)
    Initialize();

    //QueryResult* result = CharacterDatabase.PQuery("SELECT faction, standing, flags FROM character_reputation WHERE guid = '%u'", GetGUIDLow());

    if (result)
    {
        do
        {
            Field* fields = result->Fetch();

            FactionEntry const* factionEntry = sFactionStore.LookupEntry(fields[0].GetUInt16());
            if (factionEntry && factionEntry->CanHaveReputation())
            {
                FactionState* faction = &_factions[factionEntry->ReputationIndex];
                if (!faction)
                    continue;

                // Don`t save exist data
                faction->notInDB = false;
                faction->needSave = false;
                faction->needUpdateStanding = false;
                faction->needUpdateFlags = false;

                // update standing to current
                faction->Standing = fields[1].GetInt32();

                // update counters
                int32 BaseRep = GetBaseReputation(factionEntry);
                ReputationRank old_rank = GetReactionLevel(BaseRep);
                ReputationRank new_rank = GetReactionLevel(BaseRep + faction->Standing);

                uint32 dbFactionFlags = fields[2].GetUInt16();

                if (!(dbFactionFlags & FACTION_FLAG_HIDDEN))
                    UpdateRankCounters(old_rank, new_rank);

                if (dbFactionFlags & FACTION_FLAG_VISIBLE)
                    SetVisible(faction);                    // have internal checks for forced invisibility

                if (dbFactionFlags & FACTION_FLAG_INACTIVE)
                    SetInactive(faction, true);              // have internal checks for visibility requirement

                if (dbFactionFlags & FACTION_FLAG_AT_WAR)  // DB at war
                    SetAtWar(faction, true);                 // have internal checks for FACTION_FLAG_PEACE_FORCED
                else                                        // DB not at war
                {
                    // allow remove if visible (and then not FACTION_FLAG_INVISIBLE_FORCED or FACTION_FLAG_HIDDEN)
                    if (faction->Flags & FACTION_FLAG_VISIBLE)
                        SetAtWar(faction, false);            // have internal checks for FACTION_FLAG_PEACE_FORCED
                }

                // set atWar for hostile
                if (GetRank(factionEntry) <= REP_HOSTILE)
                    SetAtWar(faction, true);

                // enable war on faction Oracles/Frenzyheart Tribe
                if (GetRank(factionEntry) <= REP_HOSTILE && (factionEntry->ID == 1104 || factionEntry->ID == 1105))
                    faction->Flags |= FACTION_FLAG_AT_WAR;

                // reset changed flag if values similar to saved in DB
                if (faction->Flags != dbFactionFlags)
                {
                    faction->needSend = true;
                    faction->needUpdateFlags = true;
                }
            }
        }
        while (result->NextRow());
    }
}

ReputationMgr::ReputationMgr(Player* owner) : _player(owner), _visibleFactionCount(0), _honoredFactionCount(0), _reveredFactionCount(0), _exaltedFactionCount(0), _sendFactionIncreased(false)
{
}

void ReputationMgr::SaveToDB(SQLTransaction& trans)
{
    if (_factions.empty())
        return;

    PreparedStatement* stmt = nullptr;

    for (uint16 i = 0; i < WorldPackets::Reputation::FactionCount; ++i)
    {
        FactionState* faction = GetState(i);
        if (!faction)
            continue;

        if (faction->needSave)
        {
            stmt = CharacterDatabase.GetPreparedStatement(CHAR_REP_CHAR_REPUTATION_BY_FACTION);
            stmt->setUInt64(0, _player->GetGUIDLow());
            stmt->setUInt16(1, uint16(faction->ID));
            stmt->setInt32(2, faction->Standing);
            stmt->setUInt16(3, uint16(faction->Flags));
            trans->Append(stmt);
            faction->needSave = false;
            faction->needUpdateStanding = false;
            faction->needUpdateFlags = false;
            faction->notInDB = false;
        }
        else
        {
            if (faction->needUpdateStanding)
            {
                stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_CHAR_REP_STANDING_CHANGE);
                stmt->setInt32(0, faction->Standing);
                stmt->setUInt16(1, uint16(faction->ID));
                stmt->setUInt64(2, _player->GetGUIDLow());
                trans->Append(stmt);
                faction->needUpdateStanding = false;
            }
            if (faction->needUpdateFlags)
            {
                stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_CHAR_REP_FLAGS_CHANGE);
                stmt->setUInt16(0, uint16(faction->Flags));
                stmt->setUInt16(1, uint16(faction->ID));
                stmt->setUInt64(2, _player->GetGUIDLow());
                trans->Append(stmt);
                faction->needUpdateFlags = false;
            }
        }
    }
}

void ReputationMgr::UpdateRankCounters(ReputationRank old_rank, ReputationRank new_rank)
{
    if (old_rank >= REP_EXALTED)
        --_exaltedFactionCount;
    if (old_rank >= REP_REVERED)
        --_reveredFactionCount;
    if (old_rank >= REP_HONORED)
        --_honoredFactionCount;

    if (new_rank >= REP_EXALTED)
        ++_exaltedFactionCount;
    if (new_rank >= REP_REVERED)
        ++_reveredFactionCount;
    if (new_rank >= REP_HONORED)
        ++_honoredFactionCount;
}

void ReputationMgr::Clear()
{
    _factions.clear();
    _forcedReactions.clear();
}

uint32 ReputationMgr::GetSize()
{
    uint32 size = sizeof(ReputationMgr);

    size += _factions.size() * sizeof(FactionStateList);
    size += _forcedReactions.size() * sizeof(ForcedReactions);
    return size;
}
