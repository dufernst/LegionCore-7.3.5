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

#include "BattlePetPackets.h"
#include "WowTime.hpp"

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePet::BattlePetSlot const& petBattleSlot)
{
    data << petBattleSlot.Pet.BattlePetGUID;
    data << petBattleSlot.DisplayID;
    data << petBattleSlot.SlotIndex;
    data.WriteBit(petBattleSlot.Locked);
    data.FlushBits();

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePet::BattlePetJournalInfo const& battlePet)
{
    data << battlePet.BattlePetGUID;
    data << battlePet.SpeciesID;
    data << battlePet.CreatureID;
    data << battlePet.DisplayID;
    data << battlePet.BreedID;
    data << battlePet.Level;
    data << battlePet.Xp;
    data << battlePet.BattlePetDBFlags;
    data << battlePet.Power;
    data << battlePet.Health;
    data << battlePet.MaxHealth;
    data << battlePet.Speed;
    data << battlePet.BreedQuality;
    data.WriteBits(battlePet.CustomName.size(), 7);
    data.WriteBit(!battlePet.OwnerGuid.IsEmpty());
    data.WriteBit(battlePet.NoRename);
    data.FlushBits();

    if (!battlePet.OwnerGuid.IsEmpty())
    {
        data << battlePet.OwnerGuid;
        data << GetVirtualRealmAddress();
        data << realm.Id.Realm;
    }

    data.WriteString(battlePet.CustomName);

    return data;
}

WorldPacket const* WorldPackets::BattlePet::BattlePetJournal::Write()
{
    _worldPacket << TrapLevel;
    _worldPacket << static_cast<uint32>(Slots.size());
    _worldPacket << static_cast<uint32>(Pets.size());
    _worldPacket << NumMaxPets;
    _worldPacket.WriteBit(HasJournalLock);
    _worldPacket.FlushBits();

    for (auto const& slots : Slots)
        _worldPacket << slots;

    for (auto const& pets : Pets)
        _worldPacket << pets;

    return &_worldPacket;
}

void WorldPackets::BattlePet::Query::Read()
{
    _worldPacket >> BattlePetID;
    _worldPacket >> UnitGUID;
}

WorldPacket const* WorldPackets::BattlePet::QueryResponse::Write()
{
    _worldPacket << BattlePetID;
    _worldPacket << CreatureID;
    _worldPacket << MS::Utilities::WowTime::Encode(Timestamp);
    if (!_worldPacket.WriteBit(Allow))
        return &_worldPacket;

    _worldPacket.WriteBits(Name.size(), 8);
    _worldPacket.WriteBit(HasDeclined);
    _worldPacket.FlushBits();

    for (auto const& v : DeclinedNames)
        _worldPacket.WriteBits(v.size(), 7);

    for (auto const& v : DeclinedNames)
        _worldPacket.WriteString(v);

    _worldPacket.WriteString(Name);

    return &_worldPacket;
}

void WorldPackets::BattlePet::BattlePetGuidRead::Read()
{
    _worldPacket >> BattlePetGUID;
}

WorldPacket const* WorldPackets::BattlePet::BattlePetDeleted::Write()
{
    _worldPacket << BattlePetGUID;

    return &_worldPacket;
}

void WorldPackets::BattlePet::ModifyName::Read()
{
    _worldPacket >> BattlePetGUID;
    auto const nameLen = _worldPacket.ReadBits(7);
    if (_worldPacket.ReadBit())
    {
        int32 count[MAX_DECLINED_NAME_CASES] = {};
        for (int & var : count)
            var = _worldPacket.ReadBits(7);

        for (int32 i = 0; i < MAX_DECLINED_NAME_CASES; i++)
            DeclinedNames.name[i] = _worldPacket.ReadString(count[i]);
    }

    Name = _worldPacket.ReadString(nameLen);
}

void WorldPackets::BattlePet::SetBattleSlot::Read()
{
    _worldPacket >> BattlePetGUID;
    _worldPacket >> SlotIndex;
}

void WorldPackets::BattlePet::SetFlags::Read()
{
    _worldPacket >> BattlePetGUID;
    _worldPacket >> Flags;
    ControlType = _worldPacket.ReadBits(2);
}

WorldPacket const* WorldPackets::BattlePet::Updates::Write()
{
    _worldPacket << static_cast<uint32>(Pets.size());
    _worldPacket.FlushBits();
    _worldPacket.WriteBit(AddedPet);

    for (auto const& map : Pets)
        _worldPacket << map;

    return &_worldPacket;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePet::PetBattleEffectTarget const& effectTarget)
{
    data.FlushBits();
    data.WriteBits(effectTarget.Type, 3);

    data << effectTarget.Petx;
    switch (effectTarget.Type)
    {
        case PET_BATTLE_EFFECT_TARGET_EX_NPC_EMOTE:
            data << int32(effectTarget.Params.BroadcastTextID);
            break;
        case PET_BATTLE_EFFECT_TARGET_EX_AURA:
            data << int32(effectTarget.Params.Aura.AuraInstanceID);
            data << int32(effectTarget.Params.Aura.AuraAbilityID);
            data << int32(effectTarget.Params.Aura.RoundsRemaining);
            data << int32(effectTarget.Params.Aura.CurrentRound);
            break;
        case PET_BATTLE_EFFECT_TARGET_EX_STAT_CHANGE:
            data << int32(effectTarget.Params.NewStatValue);
            break;
        case PET_BATTLE_EFFECT_TARGET_EX_PET:
            data << int32(effectTarget.Params.Health);
            break;
        case PET_BATTLE_EFFECT_TARGET_EX_ABILITY_CHANGE:
            data << int32(effectTarget.Params.AbilityChange.ChangedAbilityID);
            data << int32(effectTarget.Params.AbilityChange.CooldownRemaining);
            data << int32(effectTarget.Params.AbilityChange.LockdownRemaining);
            break;
        case PET_BATTLE_EFFECT_TARGET_EX_TRIGGER_ABILITY:
            data << int32(effectTarget.Params.TriggerAbilityID);
            break;
        case PET_BATTLE_EFFECT_TARGET_EX_STATE:
            data << int32(effectTarget.Params.State.StateID);
            data << int32(effectTarget.Params.State.StateValue);
            break;
        default:
            break;
    }

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePet::Effect const& effect)
{
    data << effect.AbilityEffectID;
    data << effect.Flags;
    data << effect.SourceAuraInstanceID;
    data << effect.TurnInstanceID;
    data << effect.EffectType;
    data << effect.CasterPBOID;
    data << effect.StackDepth;
    data << static_cast<uint32>(effect.EffectTargetData.size());
    for (auto const& map : effect.EffectTargetData)
        data << map;

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePet::RoundResult const& roundResult)
{
    data << roundResult.CurRound;
    data << roundResult.NextPetBattleState;
    data << static_cast<uint32>(roundResult.EffectData.size());

    for (uint8 i = 0; i < PARTICIPANTS_COUNT; ++i)
    {
        data << roundResult.NextInputFlags[i];
        data << roundResult.NextTrapStatus[i];
        data << roundResult.RoundTimeSecs[i];
    }

    data << static_cast<uint32>(roundResult.Ability.size());
    for (auto const& map : roundResult.Ability)
        data << map;

    data.FlushBits();
    data.WriteBits(roundResult.PetXDied.size(), 3);

    for (auto const& map : roundResult.EffectData)
        data << map;

    for (uint8 const& map : roundResult.PetXDied)
        data << map;

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePet::FinalPet const& finalPet)
{
    data << finalPet.Guid;
    data << finalPet.Level;
    data << finalPet.Xp;
    data << finalPet.Health;
    data << finalPet.MaxHealth;
    data << finalPet.InitialLevel;
    data << finalPet.Pboid;

    data.WriteBit(finalPet.Captured);
    data.WriteBit(finalPet.SeenAction);
    data.WriteBit(finalPet.Caged);
    data.WriteBit(finalPet.AwardedXP);
    data.FlushBits();

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePet::FinalRound const& finalRound)
{
    data.FlushBits();
    data.WriteBit(finalRound.Abandoned);
    data.WriteBit(finalRound.PvpBattle);
    for (auto winner : finalRound.Winner)
        data.WriteBit(winner);

    for (auto npcCreatureID : finalRound.NpcCreatureID)
        data << npcCreatureID;

    data << static_cast<uint32>(finalRound.Pets.size());
    for (auto const& map : finalRound.Pets)
        data << map;

    return data;
}

WorldPacket const* WorldPackets::BattlePet::BattleRound::Write()
{
    _worldPacket << MsgData;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::BattlePet::PetBattleFinalRound::Write()
{
    _worldPacket << MsgData;

    return &_worldPacket;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePet::PetBattleLocation& locations)
{
    data << locations.LocationResult;
    data << locations.BattleOrigin;
    for (auto const& playerPosition : locations.PlayerPositions)
        data << playerPosition;

    return data;
}

ByteBuffer& operator>>(ByteBuffer& data, WorldPackets::BattlePet::PetBattleLocation& locations)
{
    data >> locations.LocationResult;
    data >> locations.BattleOrigin;
    for (auto & playerPosition : locations.PlayerPositions)
        data >> playerPosition;

    return data;
}

WorldPacket const* WorldPackets::BattlePet::FinalizeLocation::Write()
{
    _worldPacket << Location;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::BattlePet::PVPChallenge::Write()
{
    _worldPacket << ChallengerGUID;
    _worldPacket << Location;

    return &_worldPacket;
}

void WorldPackets::BattlePet::RequestWild::Read()
{
    _worldPacket >> Battle.TargetGUID;
    _worldPacket >> Battle.Location;
}

void WorldPackets::BattlePet::RequestPVP::Read()
{
    _worldPacket >> Battle.TargetGUID;
    _worldPacket >> Battle.Location;
}

WorldPacket const* WorldPackets::BattlePet::RequestFailed::Write()
{
    _worldPacket << Reason;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::BattlePet::SlotUpdates::Write()
{
    _worldPacket << static_cast<uint32>(Slots.size());
    for (auto const& map : Slots)
        _worldPacket << map;

    _worldPacket.WriteBit(NewSlotUnlocked);
    _worldPacket.WriteBit(AutoSlotted);
    _worldPacket.FlushBits();

    return &_worldPacket;
}

void WorldPackets::BattlePet::ReplaceFrontPet::Read()
{
    _worldPacket >> FrontPet;
}

WorldPacket const* WorldPackets::BattlePet::PetBattleQueueStatus::Write()
{
    _worldPacket << Msg.Status;
    _worldPacket << static_cast<uint32>(Msg.SlotResult.size());
    _worldPacket << Msg.Ticket;
    for (auto const& map : Msg.SlotResult)
        _worldPacket << map;

    _worldPacket.WriteBit(Msg.ClientWaitTime.is_initialized());
    _worldPacket.WriteBit(Msg.AverageWaitTime.is_initialized());
    _worldPacket.FlushBits();

    if (Msg.ClientWaitTime)
        _worldPacket << *Msg.ClientWaitTime;

    if (Msg.AverageWaitTime)
        _worldPacket << *Msg.AverageWaitTime;

    return &_worldPacket;
}

void WorldPackets::BattlePet::QueueProposeMatchResult::Read()
{
    Accepted = _worldPacket.ReadBit();
}

void WorldPackets::BattlePet::LeaveQueue::Read()
{
    _worldPacket >> Ticket;
}

void WorldPackets::BattlePet::RequestUpdate::Read()
{
    _worldPacket >> TargetGUID;
    Canceled = _worldPacket.ReadBit();
}

WorldPacket const* WorldPackets::BattlePet::GuidData::Write()
{
    _worldPacket << BattlePetGUID;

    return &_worldPacket;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePet::PetBattlePetUpdate const& update)
{
    data << update.JournalInfo.BattlePetGUID;

    data << update.JournalInfo.SpeciesID;
    data << update.JournalInfo.DisplayID;
    data << update.JournalInfo.CreatureID;

    data << update.JournalInfo.Level;
    data << update.JournalInfo.Xp;

    data << update.JournalInfo.Health;
    data << update.JournalInfo.MaxHealth;
    data << update.JournalInfo.Power;
    data << update.JournalInfo.Speed;
    data << update.NpcTeamMemberID;

    data << static_cast<uint16>(update.JournalInfo.BreedQuality);
    data << update.StatusFlags;

    data << update.Slot;

    data << static_cast<uint32>(update.Abilities.size());
    data << static_cast<uint32>(update.Auras.size());
    data << static_cast<uint32>(update.States.size());

    for (auto const& x : update.Abilities)
        data << x;

    for (auto const& v : update.Auras)
        data << v;

    for (auto const& c : update.States)
    {
        data << c.first;
        data << c.second;
    }

    data.WriteBits(update.JournalInfo.CustomName.size(), 7);
    data.WriteString(update.JournalInfo.CustomName);

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePet::BattlePetAbility const& ability)
{
    data << ability.AbilityID;
    data << ability.CooldownRemaining;
    data << ability.LockdownRemaining;
    data << ability.AbilityIndex;
    data << ability.Pboid;

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePet::PetBattlePlayerUpdate const& update)
{
    data << update.CharacterID;

    data << update.TrapAbilityID;
    data << update.TrapStatus;

    data << update.RoundTimeSecs;

    data << update.FrontPet;
    data << update.InputFlags;

    data.WriteBits(update.Pets.size(), 2);
    data.FlushBits();

    for (auto const& z : update.Pets)
        data << z;

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePet::BattlePetAura const& aura)
{
    data << aura.AbilityID;
    data << aura.InstanceID;
    data << aura.RoundsRemaining;
    data << aura.CurrentRound;
    data << aura.CasterPBOID;

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePet::PetBattleEnviroUpdate const& update)
{
    data << static_cast<uint32>(update.Auras.size());
    data << static_cast<uint32>(update.States.size());
    for (auto const& x : update.Auras)
        data << x;

    for (auto const& v : update.States)
    {
        data << v.first;
        data << v.second;
    }

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::BattlePet::PetBattleFullUpdate const& update)
{
    for (auto const& player : update.Players)
        data << player;

    for (auto const& enviro : update.Enviros)
        data << enviro;

    data << update.WaitingForFrontPetsMaxSecs;
    data << update.PvpMaxRoundTime;

    data << update.CurRound;
    data << update.NpcCreatureID;
    data << update.NpcDisplayID;

    data << update.CurPetBattleState;
    data << update.ForfeitPenalty;

    data << update.InitialWildPetGUID;

    data.WriteBit(update.IsPVP);
    data.WriteBit(update.CanAwardXP);
    data.FlushBits();

    return data;
}

WorldPacket const* WorldPackets::BattlePet::PetBattleInitialUpdate::Write()
{
    _worldPacket << MsgData;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::BattlePet::BattlePetError::Write()
{
    _worldPacket.WriteBits(Result, 4);
    _worldPacket.FlushBits();

    _worldPacket << CreatureID;

    return &_worldPacket;
}

void WorldPackets::BattlePet::PetBattleInput::Read()
{
    _worldPacket >> MoveType;
    _worldPacket >> NewFrontPet;
    _worldPacket >> DebugFlags;
    _worldPacket >> BattleInterrupted;
    _worldPacket >> AbilityID;
    _worldPacket >> Round;
    IgnoreAbandonPenalty = _worldPacket.ReadBit();
}

WorldPacket const* WorldPackets::BattlePet::UpdateMaxJournalPets::Write()
{
    _worldPacket << NumMaxPets;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::BattlePet::BattlePetTrapLevel::Write()
{
    _worldPacket << TrapLevel;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::BattlePet::BattlePetCageDateError::Write()
{
    _worldPacket << SecondsUntilCanCage;

    return &_worldPacket;
}
