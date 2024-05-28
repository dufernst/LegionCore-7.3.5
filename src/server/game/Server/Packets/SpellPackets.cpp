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

#include "SpellPackets.h"
#include "MovementPackets.h"

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Spells::SandboxScalingData const& sandboxScalingData)
{
    data.WriteBits(sandboxScalingData.Type, 4);
    data.FlushBits();

    data << int16(sandboxScalingData.PlayerLevelDelta);
    data << uint16(sandboxScalingData.PlayerItemLevel);

    data << uint8(sandboxScalingData.TargetLevel);
    data << uint8(sandboxScalingData.Expansion);
    data << uint8(sandboxScalingData.Class);
    data << uint8(sandboxScalingData.TargetMinScalingLevel);
    data << uint8(sandboxScalingData.TargetMaxScalingLevel);
    data << int8(sandboxScalingData.TargetScalingLevelDelta);

    return data;
}

namespace WorldPackets
{
    namespace Spells
    {
        template<class T, class U>
        bool SandboxScalingData::GenerateDataForUnits(T* /*attacker*/, U* /*target*/)
        {
            return false;
        }

        template<>
        bool SandboxScalingData::GenerateDataForUnits<Creature, Player>(Creature* attacker, Player* target)
        {
            return false;

            CreatureTemplate const* creatureTemplate = attacker->GetCreatureTemplate();

            Type = TYPE_CREATURE_TO_PLAYER_DAMAGE;
            PlayerLevelDelta = target->GetInt32Value(PLAYER_FIELD_SCALING_PLAYER_LEVEL_DELTA);
            PlayerItemLevel = target->GetAverageItemLevelEquipped();
            TargetLevel = target->getLevel();
            Expansion = creatureTemplate->RequiredExpansion;
            Class = creatureTemplate->unit_class;
            TargetMinScalingLevel = creatureTemplate->ScaleLevelMin;
            TargetMaxScalingLevel = creatureTemplate->ScaleLevelMax;
            TargetScalingLevelDelta = int8(attacker->GetInt32Value(UNIT_FIELD_SCALING_LEVEL_DELTA));
            return true;
        }

        template<>
        bool SandboxScalingData::GenerateDataForUnits<Player, Creature>(Player* attacker, Creature* target)
        {
            return false;

            CreatureTemplate const* creatureTemplate = target->GetCreatureTemplate();

            Type = TYPE_PLAYER_TO_CREATURE_DAMAGE;
            PlayerLevelDelta = attacker->GetInt32Value(PLAYER_FIELD_SCALING_PLAYER_LEVEL_DELTA);
            PlayerItemLevel = attacker->GetAverageItemLevelEquipped();
            TargetLevel = target->getLevel();
            Expansion = creatureTemplate->RequiredExpansion;
            Class = creatureTemplate->unit_class;
            TargetMinScalingLevel = creatureTemplate->ScaleLevelMin;
            TargetMaxScalingLevel = creatureTemplate->ScaleLevelMax;
            TargetScalingLevelDelta = int8(target->GetInt32Value(UNIT_FIELD_SCALING_LEVEL_DELTA));
            return true;
        }

        template<>
        bool SandboxScalingData::GenerateDataForUnits<Creature, Creature>(Creature* attacker, Creature* target)
        {
            return false;

            CreatureTemplate const* creatureTemplate = target->GetCreatureTemplate();

            Type = TYPE_CREATURE_TO_CREATURE_DAMAGE;
            PlayerLevelDelta = 0;
            PlayerItemLevel = 0;
            TargetLevel = target->getLevel();
            Expansion = creatureTemplate->RequiredExpansion;
            Class = creatureTemplate->unit_class;
            TargetMinScalingLevel = creatureTemplate->ScaleLevelMin;
            TargetMaxScalingLevel = creatureTemplate->ScaleLevelMax;
            TargetScalingLevelDelta = int8(attacker->GetInt32Value(UNIT_FIELD_SCALING_LEVEL_DELTA));
            return true;
        }

        template<>
        bool SandboxScalingData::GenerateDataForUnits<Unit, Unit>(Unit* attacker, Unit* target)
        {
            return false;

            if (Player* playerAttacker = attacker->ToPlayer())
            {
                if (Player* playerTarget = target->ToPlayer())
                    return GenerateDataForUnits(playerAttacker, playerTarget);
                else if (Creature* creatureTarget = target->ToCreature())
                    return GenerateDataForUnits(playerAttacker, creatureTarget);
            }
            else if (Creature* creatureAttacker = attacker->ToCreature())
            {
                if (Player* playerTarget = target->ToPlayer())
                    return GenerateDataForUnits(creatureAttacker, playerTarget);
                else if (Creature* creatureTarget = target->ToCreature())
                    return GenerateDataForUnits(creatureAttacker, creatureTarget);
            }

            return false;
        }
    }
}

void WorldPackets::Spells::CancelAura::Read()
{
    _worldPacket >> SpellID;
    _worldPacket >> CasterGUID;
}

WorldPackets::Spells::CategoryCooldown::CategoryCooldownInfo::CategoryCooldownInfo(uint32 category, int32 cooldown): Category(category), ModCooldown(cooldown) { }

WorldPacket const* WorldPackets::Spells::CategoryCooldown::Write()
{
    _worldPacket.reserve(4 + 8 * CategoryCooldowns.size());

    _worldPacket << static_cast<uint32>(CategoryCooldowns.size());

    for (CategoryCooldownInfo const& cooldown : CategoryCooldowns)
    {
        _worldPacket << uint32(cooldown.Category);
        _worldPacket << int32(cooldown.ModCooldown);
    }

    return &_worldPacket;
}

void WorldPackets::Spells::CancelChannelling::Read()
{
    _worldPacket >> ChannelSpell;
}

WorldPacket const* WorldPackets::Spells::SendKnownSpells::Write()
{
    _worldPacket.reserve(1 + 4 * KnownSpells.size() + 4 * FavoriteSpells.size());

    _worldPacket.WriteBit(InitialLogin);
    _worldPacket << static_cast<uint32>(KnownSpells.size());
    _worldPacket << static_cast<uint32>(FavoriteSpells.size());

    for (uint32 const& spellId : KnownSpells)
        _worldPacket << spellId;

    for (uint32 const& spellId : FavoriteSpells)
        _worldPacket << spellId;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Spells::UpdateActionButtons::Write()
{
    for (auto actionButton : ActionButtons)
        _worldPacket << actionButton;

    _worldPacket << Reason;

    return &_worldPacket;
}

void WorldPackets::Spells::SetActionButton::Read()
{
    _worldPacket >> Action;
    _worldPacket >> Type;
    _worldPacket >> Index;
}

WorldPacket const* WorldPackets::Spells::SendUnlearnSpells::Write()
{
    _worldPacket << static_cast<uint32>(Spells.size());
    for (uint32 const& spellId : Spells)
        _worldPacket << uint32(spellId);

    return &_worldPacket;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Spells::AuraDataInfo const& auraData)
{
    data << auraData.CastGuid;
    data << int32(auraData.SpellID);
    data << int32(auraData.SpellXSpellVisualID);
    data << uint8(auraData.Flags);
    data << uint32(auraData.ActiveFlags);
    data << uint16(auraData.CastLevel);
    data << uint8(auraData.Applications);
    data.WriteBit(auraData.CastUnit.is_initialized());
    data.WriteBit(auraData.Duration.is_initialized());
    data.WriteBit(auraData.Remaining.is_initialized());
    data.WriteBit(auraData.TimeMod.is_initialized());
    data.WriteBits(auraData.EstimatedPoints.size(), 6);
    data.WriteBits(auraData.Points.size(), 6);
    data.WriteBit(auraData.SandboxScaling.is_initialized());
    data.FlushBits();

    if (auraData.SandboxScaling)
        data << *auraData.SandboxScaling;

    if (auraData.CastUnit)
        data << *auraData.CastUnit;

    if (auraData.Duration)
        data << uint32(*auraData.Duration);

    if (auraData.Remaining)
        data << uint32(*auraData.Remaining);

    if (auraData.TimeMod)
        data << float(*auraData.TimeMod);
    
    if (!auraData.EstimatedPoints.empty())
        data.append(auraData.EstimatedPoints.data(), auraData.EstimatedPoints.size());

    if (!auraData.Points.empty())
        data.append(auraData.Points.data(), auraData.Points.size());

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Spells::AuraInfo const& aura)
{
    data << aura.Slot;
    data.WriteBit(aura.AuraData.is_initialized());
    data.FlushBits();

    if (aura.AuraData)
        data << *aura.AuraData;

    return data;
}

WorldPacket const* WorldPackets::Spells::AuraUpdate::Write()
{
    _worldPacket.WriteBit(UpdateAll);
    _worldPacket.WriteBits(Auras.size(), 9);
    _worldPacket.FlushBits();

    for (AuraInfo const& aura : Auras)
        _worldPacket << aura;

    _worldPacket << UnitGUID;

    return &_worldPacket;
}

ByteBuffer& operator>>(ByteBuffer& buffer, WorldPackets::Spells::TargetLocation& location)
{
    buffer >> location.Transport;
    buffer >> location.Location.m_positionX;
    buffer >> location.Location.m_positionY;
    buffer >> location.Location.m_positionZ;

    return buffer;
}

ByteBuffer& operator>>(ByteBuffer& buffer, Optional<WorldPackets::Spells::TargetLocation>& location)
{
    location = boost::in_place();
    buffer >> location->Transport;
    buffer >> location->Location.m_positionX;
    buffer >> location->Location.m_positionY;
    buffer >> location->Location.m_positionZ;

    return buffer;
}

ByteBuffer& operator>>(ByteBuffer& buffer, WorldPackets::Spells::SpellTargetData& targetData)
{
    buffer.ResetBitPos();

    targetData.Flags = buffer.ReadBits(25);
    bool const hasSrcLocation = buffer.ReadBit();
    bool const hasDstLocation = buffer.ReadBit();
    bool const hasOrientation = buffer.ReadBit();
    bool const hasMapID = buffer.ReadBit();
    uint32 nameLength = buffer.ReadBits(7);

    buffer >> targetData.Unit;
    buffer >> targetData.Item;

    if (hasSrcLocation)
        buffer >> targetData.SrcLocation;

    if (hasDstLocation)
        buffer >> targetData.DstLocation;

    if (hasOrientation)
        targetData.Orientation = buffer.read<float>();

    if (hasMapID)
        targetData.MapID = buffer.read<uint32>();

    targetData.Name = buffer.ReadString(nameLength);

    return buffer;
}

ByteBuffer& operator>>(ByteBuffer& buffer, WorldPackets::Spells::MissileTrajectoryRequest& trajectory)
{
    buffer >> trajectory.Pitch;
    buffer >> trajectory.Speed;

    return buffer;
}

ByteBuffer& operator>>(ByteBuffer& buffer, WorldPackets::Spells::SpellCastRequest& request)
{
    buffer >> request.SpellGuid;
    for (auto& misc : request.Misc)
        buffer >> misc;
    buffer >> request.SpellID;
    buffer >> request.SpellXSpellVisualID;
    buffer >> request.MissileTrajectory;
    buffer >> request.Charmer;

    buffer.ResetBitPos();
    request.SendCastFlags = buffer.ReadBits(5);
    bool const hasMoveUpdate = buffer.ReadBit();
    request.Weight.resize(buffer.ReadBits(2));

    buffer >> request.Target;

    if (hasMoveUpdate)
    {
        MovementInfo movementInfo;
        buffer >> movementInfo;
        request.MoveUpdate = movementInfo;
    }
        //request.MoveUpdate = buffer.read<MovementInfo>();

    for (WorldPackets::Spells::SpellWeight& weight : request.Weight)
    {
        buffer.ResetBitPos();
        weight.Type = buffer.ReadBits(2);
        buffer >> weight.ID;
        buffer >> weight.Quantity;
    }

    return buffer;
}

void WorldPackets::Spells::CastSpell::Read()
{
    _worldPacket >> Cast;
}

void WorldPackets::Spells::PetCastSpell::Read()
{
    _worldPacket >> PetGUID;
    _worldPacket >> Cast;
}

void WorldPackets::Spells::ItemUse::Read()
{
    _worldPacket >> bagIndex;
    _worldPacket >> slot;
    _worldPacket >> itemGUID;
    _worldPacket >> Cast;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Spells::TargetLocation const& targetLocation)
{
    data << targetLocation.Transport;
    data << targetLocation.Location.m_positionX;
    data << targetLocation.Location.m_positionY;
    data << targetLocation.Location.m_positionZ;

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Spells::SpellTargetData const& spellTargetData)
{
    data.WriteBits(spellTargetData.Flags, 25);
    data.WriteBit(spellTargetData.SrcLocation.is_initialized());
    data.WriteBit(spellTargetData.DstLocation.is_initialized());
    data.WriteBit(spellTargetData.Orientation.is_initialized());
    data.WriteBit(spellTargetData.MapID.is_initialized());
    data.WriteBits(spellTargetData.Name.size(), 7);
    data.FlushBits();

    data << spellTargetData.Unit;
    data << spellTargetData.Item;

    if (spellTargetData.SrcLocation)
        data << *spellTargetData.SrcLocation;

    if (spellTargetData.DstLocation)
        data << *spellTargetData.DstLocation;

    if (spellTargetData.Orientation)
        data << *spellTargetData.Orientation;

    if (spellTargetData.MapID)
        data << *spellTargetData.MapID;

    data.WriteString(spellTargetData.Name);

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Spells::SpellMissStatus const& spellMissStatus)
{
    data.WriteBits(spellMissStatus.Reason, 4);
    if (spellMissStatus.Reason == SPELL_MISS_REFLECT)
        data.WriteBits(spellMissStatus.ReflectStatus, 4);
    data.FlushBits();

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Spells::SpellPowerData const& spellPowerData)
{
    data << spellPowerData.Remain;
    data << spellPowerData.Power;

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Spells::RuneData const& runeData)
{
    data << runeData.Start;
    data << runeData.Count;
    data << static_cast<uint32>(runeData.Cooldowns.size());
    for (uint8 const& cd : runeData.Cooldowns)
        data << cd;

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Spells::MissileTrajectoryResult const& missileTrajectory)
{
    data << missileTrajectory.TravelTime;
    data << missileTrajectory.Pitch;

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Spells::SpellAmmo const& spellAmmo)
{
    data << spellAmmo.DisplayID;

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Spells::CreatureImmunities const& immunities)
{
    data << immunities.School;
    data << immunities.Value;

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Spells::SpellHealPrediction const& spellPred)
{
    data << spellPred.Points;
    data << spellPred.Type;
    data << spellPred.BeaconGUID;

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Spells::SpellCastData const& spellCastData)
{
    data << spellCastData.CasterGUID;
    data << spellCastData.CasterUnit;
    data << spellCastData.CastGuid;
    data << spellCastData.CastGuid2;

    data << int32(spellCastData.SpellID);
    data << uint32(spellCastData.SpellXSpellVisualID);
    data << uint32(spellCastData.CastFlags);
    data << uint32(spellCastData.CastTime);
    data << spellCastData.MissileTrajectory;
    data << spellCastData.Ammo;
    data << uint8(spellCastData.DestLocSpellCastIndex);
    data << spellCastData.Immunities;
    data << spellCastData.Predict;
    data.WriteBits(spellCastData.CastFlagsEx, 23);
    data.WriteBits(spellCastData.HitTargets.size(), 16);
    data.WriteBits(spellCastData.MissTargets.size(), 16);
    data.WriteBits(spellCastData.MissStatus.size(), 16);
    data.WriteBits(spellCastData.RemainingPower.size(), 9);
    data.WriteBit(spellCastData.RemainingRunes.is_initialized());
    data.WriteBits(spellCastData.TargetPoints.size(), 16);
    data.FlushBits();

    for (WorldPackets::Spells::SpellMissStatus const& status : spellCastData.MissStatus)
        data << status;

    data << spellCastData.Target;

    for (ObjectGuid const& target : spellCastData.HitTargets)
        data << target;

    for (ObjectGuid const& target : spellCastData.MissTargets)
        data << target;

    for (WorldPackets::Spells::SpellPowerData const& power : spellCastData.RemainingPower)
        data << power;

    if (spellCastData.RemainingRunes)
        data << *spellCastData.RemainingRunes;

    for (WorldPackets::Spells::TargetLocation const& targetLoc : spellCastData.TargetPoints)
        data << targetLoc;

    return data;
}

WorldPacket const* WorldPackets::Spells::SpellStart::Write()
{
    _worldPacket << Cast;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Spells::SpellGo::Write()
{
    *this << Cast;

    WriteLogDataBit();
    FlushBits();

    WriteLogData();

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Spells::LearnedSpells::Write()
{
    _worldPacket << static_cast<uint32>(SpellID.size());
    _worldPacket << static_cast<uint32>(FavoriteSpellID.size());
    for (int32 const& spell : SpellID)
        _worldPacket << spell;
    for (int32 const& spell : FavoriteSpellID)
        _worldPacket << spell;

    _worldPacket.WriteBit(SuppressMessaging);
    _worldPacket.FlushBits();

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Spells::SpellFailure::Write()
{
    _worldPacket << CasterUnit;
    _worldPacket << CastGUID;
    _worldPacket << SpellID;
    _worldPacket << SpellXSpellVisualID;
    _worldPacket << Reason;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Spells::SpellFailedOther::Write()
{
    _worldPacket << CasterUnit;
    _worldPacket << CastGUID;
    _worldPacket << SpellID;
    _worldPacket << SpellXSpellVisualID;
    _worldPacket << Reason;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Spells::CastFailed::Write()
{
    _worldPacket << SpellGuid;
    _worldPacket << SpellID;
    _worldPacket << SpellXSpellVisualID;
    _worldPacket << Reason;
    _worldPacket << FailedArg1;
    if (_worldPacket.GetOpcode() != SMSG_PET_CAST_FAILED)
        _worldPacket << FailedArg2;

    return &_worldPacket;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Spells::SpellModifierData const& spellModifierData)
{
    data << spellModifierData.ModifierValue;
    data << spellModifierData.ClassIndex;

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Spells::SpellModifier const& spellModifier)
{
    data << spellModifier.ModIndex;
    data << static_cast<uint32>(spellModifier.ModifierData.size());
    for (WorldPackets::Spells::SpellModifierData const& modData : spellModifier.ModifierData)
        data << modData;

    return data;
}

WorldPacket const* WorldPackets::Spells::SetSpellModifier::Write()
{
    _worldPacket << static_cast<uint32>(Modifiers.size());
    for (SpellModifier const& spellMod : Modifiers)
        _worldPacket << spellMod;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Spells::UnlearnedSpells::Write()
{
    _worldPacket << uint32(SpellID.size());
    for (uint32 const& spellId : SpellID)
        _worldPacket << uint32(spellId);

    _worldPacket.WriteBit(SuppressMessaging);
    _worldPacket.FlushBits();

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Spells::SupercededSpells::Write()
{
    _worldPacket << uint32(SpellID.size());
    _worldPacket << uint32(Superceded.size());
    _worldPacket << uint32(FavoriteSpellID.size());

    if (!SpellID.empty())
        _worldPacket.append(SpellID.data(), SpellID.size());

    if (!Superceded.empty())
        _worldPacket.append(Superceded.data(), Superceded.size());

    if (!FavoriteSpellID.empty())
        _worldPacket.append(FavoriteSpellID.data(), FavoriteSpellID.size());

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Spells::CooldownEvent::Write()
{
    _worldPacket << int32(SpellID);
    _worldPacket.WriteBit(IsPet);
    _worldPacket.FlushBits();

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Spells::ClearCooldowns::Write()
{
    _worldPacket << static_cast<uint32>(SpellID.size());
    if (!SpellID.empty())
        _worldPacket.append(SpellID.data(), SpellID.size());

    _worldPacket.WriteBit(IsPet);
    _worldPacket.FlushBits();

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Spells::ClearCooldown::Write()
{
    _worldPacket << uint32(SpellID);
    _worldPacket.WriteBit(ClearOnHold);
    _worldPacket.WriteBit(IsPet);
    _worldPacket.FlushBits();

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Spells::ModifyCooldown::Write()
{
    _worldPacket << int32(SpellID);
    _worldPacket << int32(DeltaTime);
    _worldPacket.WriteBit(IsPet);
    _worldPacket.FlushBits();

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Spells::ModifyCooldownRecoverySpeed::Write()
{
    _worldPacket << int32(SpellID);
    _worldPacket << Multiplier;
    _worldPacket << Multiplier2;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Spells::ModifyChargeRecoverySpeed::Write()
{
    _worldPacket << int32(SpellID);
    _worldPacket << Multiplier;
    _worldPacket << Multiplier2;
    _worldPacket.WriteBit(IsPet);
    _worldPacket.FlushBits();

    return &_worldPacket;
}

WorldPackets::Spells::SpellCooldownStruct::SpellCooldownStruct(uint32 spellId, uint32 forcedCooldown) : SpellID(spellId), ForcedCooldown(forcedCooldown) { }
WorldPackets::Spells::SpellCooldownStruct::SpellCooldownStruct(uint32 spellId, uint32 forcedCooldown, float modRate): SpellID(spellId), ForcedCooldown(forcedCooldown), ModRate(modRate) { }

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Spells::SpellCooldownStruct const& cooldown)
{
    data << uint32(cooldown.SpellID);
    data << uint32(cooldown.ForcedCooldown);
    data << float(cooldown.ModRate);

    return data;
}

WorldPacket const* WorldPackets::Spells::SpellCooldown::Write()
{
    _worldPacket << Caster;
    _worldPacket << uint8(Flags);
    _worldPacket << static_cast<uint32>(SpellCooldowns.size());
    for (SpellCooldownStruct const& cooldown : SpellCooldowns)
        _worldPacket << cooldown;

    return &_worldPacket;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Spells::SpellHistoryEntry const& historyEntry)
{
    data << uint32(historyEntry.SpellID);
    data << uint32(historyEntry.ItemID);
    if (historyEntry.OnHold)
    {
        data << uint32(historyEntry.Category);
        data << int32(historyEntry.RecoveryTime);
        data << int32(historyEntry.CategoryRecoveryTime);
    }
    else
    {
        data << uint32(0);
        data << int32(historyEntry.RecoveryTime);
        data << int32(0);
    }

    data << float(historyEntry.ModRate);

    data.WriteBit(historyEntry.unused622_1.is_initialized());
    data.WriteBit(historyEntry.unused622_2.is_initialized());
    data.WriteBit(historyEntry.OnHold);
    data.FlushBits();

    if (historyEntry.unused622_1)
        data << *historyEntry.unused622_1;
    if (historyEntry.unused622_2)
        data << *historyEntry.unused622_2;

    return data;
}

WorldPacket const* WorldPackets::Spells::SendSpellHistory::Write()
{
    _worldPacket << static_cast<uint32>(Entries.size());
    for (SpellHistoryEntry const& historyEntry : Entries)
        _worldPacket << historyEntry;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Spells::RefreshSpellHistory::Write()
{
    _worldPacket << static_cast<uint32>(Entries.size());
    for (SpellHistoryEntry const& historyEntry : Entries)
        _worldPacket << historyEntry;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Spells::ClearAllSpellCharges::Write()
{
    _worldPacket.WriteBit(IsPet);
    _worldPacket.FlushBits();

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Spells::ClearSpellCharges::Write()
{
    _worldPacket << int32(Category);
    _worldPacket.WriteBit(IsPet);
    _worldPacket.FlushBits();

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Spells::SetSpellCharges::Write()
{
    _worldPacket << int32(Category);
    _worldPacket << uint32(NextRecoveryTime);
    _worldPacket << uint8(ConsumedCharges);
    _worldPacket << float(ChargeModRate);
    _worldPacket.WriteBit(IsPet);
    _worldPacket.FlushBits();

    return &_worldPacket;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Spells::SpellChargeEntry const& chargeEntry)
{
    data << uint32(chargeEntry.Category);
    data << uint32(chargeEntry.NextRecoveryTime);
    data << float(chargeEntry.ChargeModRate);
    data << uint8(chargeEntry.ConsumedCharges);

    return data;
}

WorldPacket const* WorldPackets::Spells::SendSpellCharges::Write()
{
    _worldPacket << static_cast<uint32>(Entries.size());
    for (SpellChargeEntry const& chargeEntry : Entries)
        _worldPacket << chargeEntry;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Spells::ClearTarget::Write()
{
    _worldPacket << Guid;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Spells::CancelOrphanSpellVisual::Write()
{
    _worldPacket << int32(SpellVisualID);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Spells::CancelSpellVisual::Write()
{
    _worldPacket << Source;
    _worldPacket << int32(SpellVisualID);

    return &_worldPacket;
}

void WorldPackets::Spells::CancelCast::Read()
{
    _worldPacket >> CastGuid;
    _worldPacket >> SpellID;
}

void WorldPackets::Spells::OpenItem::Read()
{
    _worldPacket >> Slot;
    _worldPacket >> PackSlot;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Spells::SpellChannelStartInterruptImmunities const& interruptImmunities)
{
    data << int32(interruptImmunities.SchoolImmunities);
    data << int32(interruptImmunities.Immunities);

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Spells::SpellTargetedHealPrediction const& targetedHealPrediction)
{
    data << targetedHealPrediction.TargetGUID;
    data << targetedHealPrediction.Predict;

    return data;
}

WorldPacket const* WorldPackets::Spells::SpellChannelStart::Write()
{
    _worldPacket << CasterGUID;
    _worldPacket << int32(SpellID);
    _worldPacket << int32(SpellXSpellVisualID);
    _worldPacket << uint32(ChannelDuration);
    _worldPacket.WriteBit(InterruptImmunities.is_initialized());
    _worldPacket.WriteBit(HealPrediction.is_initialized());
    _worldPacket.FlushBits();

    if (InterruptImmunities)
        _worldPacket << *InterruptImmunities;

    if (HealPrediction)
        _worldPacket << *HealPrediction;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Spells::SpellChannelUpdate::Write()
{
    _worldPacket << CasterGUID;
    _worldPacket << int32(TimeRemaining);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Spells::ResurrectRequest::Write()
{
    _worldPacket << ResurrectOffererGUID;
    _worldPacket << ResurrectOffererVirtualRealmAddress;

    _worldPacket << PetNumber;
    _worldPacket << SpellID;

    _worldPacket.WriteBits(Name.length(), 11);
    _worldPacket.WriteBit(UseTimer);
    _worldPacket.WriteBit(Sickness);
    _worldPacket.WriteString(Name);

    return &_worldPacket;
}

void WorldPackets::Spells::SelfRes::Read()
{
    _worldPacket >> SpellID;
}

void WorldPackets::Spells::UnlearnSkill::Read()
{
    _worldPacket >> SkillID;
}

void WorldPackets::Spells::UnlearnSpecialization::Read()
{
    _worldPacket >> SpecializationIndex;
}

void WorldPackets::Spells::GetMirrorImageData::Read()
{
    _worldPacket >> UnitGUID;
    _worldPacket >> DisplayID;
}

WorldPacket const* WorldPackets::Spells::MirrorImageComponentedData::Write()
{
    _worldPacket << UnitGUID;
    _worldPacket << DisplayID;

    _worldPacket << RaceID;
    _worldPacket << Gender;
    _worldPacket << ClassID;
    _worldPacket << BeardVariation;
    _worldPacket << FaceVariation;
    _worldPacket << HairVariation;
    _worldPacket << HairColor;
    _worldPacket << SkinColor;
    _worldPacket.append(CustomDisplay.data(), CustomDisplay.size());

    _worldPacket << GuildGUID;
    _worldPacket << static_cast<uint32>(ItemDisplayID.size());
    for (auto const& itemDisplayId : ItemDisplayID)
        _worldPacket << itemDisplayId;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Spells::MirrorImageCreatureData::Write()
{
    _worldPacket << UnitGUID;
    _worldPacket << DisplayID;

    return &_worldPacket;
}

void WorldPackets::Spells::SpellClick::Read()
{
    _worldPacket >> SpellClickUnitGuid;
    TryAutoDismount = _worldPacket.ReadBit();
}

WorldPacket const* WorldPackets::Spells::ResyncRunes::Write()
{
    _worldPacket << uint8(Start);
    _worldPacket << uint8(Count);
    _worldPacket << static_cast<uint32>(Cooldowns.size());
    if (!Cooldowns.empty())
        _worldPacket.append(Cooldowns.data(), Cooldowns.size());

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Spells::ScriptCast::Write()
{
    _worldPacket << SpellID;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Spells::PlaySpellVisual::Write()
{
    _worldPacket << Source;
    _worldPacket << Target;
    _worldPacket << TargetPosition.PositionXYZStream();
    _worldPacket << SpellVisualID;
    _worldPacket << TravelSpeed;
    _worldPacket << MissReason;
    _worldPacket << ReflectStatus;
    _worldPacket << TargetPosition.GetOrientation();
    _worldPacket.WriteBit(SpeedAsTime);
    _worldPacket.FlushBits();

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Spells::PlaySpellVisualKit::Write()
{
    _worldPacket << Unit;
    _worldPacket << KitType;
    _worldPacket << Duration;
    _worldPacket << KitRecID;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Spells::PlayOrphanSpellVisual::Write()
{
    _worldPacket << SourceLocation;
    _worldPacket << SourceRotation;
    _worldPacket << TargetLocation;
    _worldPacket << Target;
    _worldPacket << SpellVisualID;
    _worldPacket << TravelSpeed;
    _worldPacket << UnkFloat;
    _worldPacket.WriteBit(SpeedAsTime);
    _worldPacket.FlushBits();

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Spells::WeeklySpellUsage::Write()
{
    _worldPacket << static_cast<uint32>(Category.size());
    for (int32 const& x : Category)
        _worldPacket << x;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Spells::ResumeCastBar::Write()
{
    _worldPacket << Guid;
    _worldPacket << Target;
    _worldPacket << SpellID;
    _worldPacket << SpellXSpellVisualID;
    _worldPacket << TimeRemaining;
    _worldPacket << TotalTime;
    _worldPacket.WriteBit(Immunities.is_initialized());
    if (Immunities)
        _worldPacket << *Immunities;

    return &_worldPacket;
}

void WorldPackets::Spells::MissileTrajectoryCollision::Read()
{
    _worldPacket >> Target;
    _worldPacket >> SpellID;
    _worldPacket >> CastGUID;
    _worldPacket >> CollisionPos;
}

WorldPacket const* WorldPackets::Spells::NotifyMissileTrajectoryCollision::Write()
{
    _worldPacket << Caster;
    _worldPacket << CastGUID;
    _worldPacket << CollisionPos;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Spells::NotifyDestLocSpellCast::Write()
{
    _worldPacket << Caster;
    _worldPacket << DestTransport;
    _worldPacket << SpellID;
    _worldPacket << SourceLoc;
    _worldPacket << DestLoc;
    _worldPacket << MissileTrajectoryPitch;
    _worldPacket << MissileTrajectorySpeed;
    _worldPacket << TravelTime;
    _worldPacket << DestLocSpellCastIndex;
    _worldPacket << CastID;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Spells::LossOfControlAuraUpdate::Write()
{
    _worldPacket << TargetGuid;
    _worldPacket << static_cast<uint32>(Infos.size());
    for (auto const& x : Infos)
    {
        _worldPacket << x.AuraSlot;
        _worldPacket << x.EffectIndex;
        _worldPacket.WriteBits(x.Type, 8);
        _worldPacket.WriteBits(x.Mechanic, 8);
    }

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Spells::CancelSpellVisualKit::Write()
{
    _worldPacket << Source;
    _worldPacket << SpellVisualKitID;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Spells::AuraPointsDepleted::Write()
{
    _worldPacket << Unit;
    _worldPacket << Slot;
    _worldPacket << EffectIndex;

    return &_worldPacket;
}

ByteBuffer& operator<<(ByteBuffer& data, AreaTriggerSpline const& triggerSplineData)
{
    data << triggerSplineData.TimeToTarget;
    data << triggerSplineData.ElapsedTimeForMovement;
    data.FlushBits();

    data.WriteBits(triggerSplineData.VerticesPoints.size(), 16);
    for (auto& x : triggerSplineData.VerticesPoints)
    {
        data << x.x;
        data << x.y;
        data << x.z;
    }

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, AreaTriggerCircle const& triggerCircleData)
{
    data.FlushBits();
    data.WriteBit(triggerCircleData.PathTarget.is_initialized());
    data.WriteBit(triggerCircleData.Center.is_initialized());
    data.WriteBit(triggerCircleData.CounterClockwise);
    data.WriteBit(triggerCircleData.CanLoop);

    data << triggerCircleData.TimeToTarget;
    data << triggerCircleData.ElapsedTimeForMovement;
    data << triggerCircleData.StartDelay;
    data << triggerCircleData.Radius;
    data << triggerCircleData.BlendFromRadius;
    data << triggerCircleData.InitialAngle;
    data << triggerCircleData.ZOffset;

    if (triggerCircleData.PathTarget)
        data << *triggerCircleData.PathTarget;

    if (triggerCircleData.Center)
        data << *triggerCircleData.Center;

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, AreaTriggerPolygon const& polygon)
{
    data << static_cast<uint32>(polygon.Vertices.size());
    data << static_cast<uint32>(polygon.VerticesTarget.size());
    data << polygon.Height;
    data << polygon.HeightTarget;

    for (auto const& v : polygon.Vertices)
        data << v;

    for (auto const& v : polygon.VerticesTarget)
        data << v;

    return data;
}

WorldPacket const* WorldPackets::Spells::AreaTriggerMoveScale::Write()
{
    _worldPacket << TriggerGUID;
    _worldPacket << Spline;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Spells::AreaTriggerReShape::Write()
{
    _worldPacket << TriggerGUID;
    _worldPacket.FlushBits();
    _worldPacket.WriteBit(Spline.is_initialized());
    _worldPacket.WriteBit(Circle.is_initialized());

    if (Spline)
        _worldPacket << *Spline;

    if (Circle)
        _worldPacket << *Circle;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Spells::AreaTriggerSplineUnk2::Write()
{
    _worldPacket << TriggerGUID;
    _worldPacket << UnkPosition;
    _worldPacket << Spline;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Spells::AreaTriggerSequence::Write()
{
    _worldPacket << TriggerGUID;
    _worldPacket << SequenceAnimationID;
    _worldPacket.WriteBit(SequenceEntered);
    _worldPacket.FlushBits();

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Spells::AreaTriggerDenied::Write()
{
    _worldPacket << SequenceAnimationID;
    _worldPacket.WriteBit(SequenceEntered);
    _worldPacket.FlushBits();

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Spells::SpellDelayed::Write()
{
    _worldPacket << Caster;
    _worldPacket << ActualDelay;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Spells::MissileCancel::Write()
{
    _worldPacket << OwnerGUID;
    _worldPacket << SpellID;
    _worldPacket.WriteBit(Reverse);
    _worldPacket.FlushBits();

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Spells::DispelFailed::Write()
{
    _worldPacket << CasterGUID;
    _worldPacket << VictimGUID;
    _worldPacket << SpellID;
    _worldPacket << static_cast<uint32>(FailedSpellIDs.size());
    for (uint32 const& x : FailedSpellIDs)
        _worldPacket << x;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Spells::SpellDispellLog::Write()
{
    _worldPacket.WriteBit(IsSteal);
    _worldPacket.WriteBit(IsBreak);
    _worldPacket.FlushBits();
    _worldPacket << TargetGUID;
    _worldPacket << CasterGUID;
    _worldPacket << SpellID;
    _worldPacket << static_cast<uint32>(Dispell.size());
    for (auto const& x : Dispell)
    {
        _worldPacket << x.SpellID;
        _worldPacket.WriteBit(x.IsHarmful);
        _worldPacket.WriteBit(x.Rolled.is_initialized());
        _worldPacket.WriteBit(x.Needed.is_initialized());
        _worldPacket.FlushBits();
        if (x.Rolled)
            _worldPacket << *x.Rolled;
        if (x.Needed)
            _worldPacket << *x.Needed;
    }

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Spells::SpellAbsorbLog::Write()
{
    _worldPacket << Victim;
    _worldPacket << Caster;
    _worldPacket << InterruptedSpellID;
    _worldPacket << SpellID;
    _worldPacket << ShieldTargetGUID;
    _worldPacket << Absorbed;
    _worldPacket.FlushBits();
    if (_worldPacket.WriteBit(LogData.is_initialized()))
        _worldPacket << *LogData;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Spells::SpellCategoryCooldown::Write()
{
    _worldPacket << Caster;
    _worldPacket << SpellID;
    _worldPacket.FlushBits();
    _worldPacket << UnkBit;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Spells::AddLossOfControl::Write()
{
    _worldPacket << Target;
    _worldPacket << SpellID;
    _worldPacket << Caster;
    _worldPacket << Duration;
    _worldPacket << DurationRemaining;
    _worldPacket << LockoutSchoolMask;
    _worldPacket.WriteBits(Type, 8);
    _worldPacket.WriteBits(Mechanic, 8);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Spells::RemoveLossOfControl::Write()
{
    _worldPacket.WriteBits(Type, 8);
    _worldPacket << SpellID;
    _worldPacket << Caster;

    return &_worldPacket;
}

void WorldPackets::Spells::UpdateMissileTrajectory::Read()
{
    _worldPacket >> Guid;
    _worldPacket >> MoveMsgID;
    _worldPacket >> SpellID;
    _worldPacket >> Pitch;
    _worldPacket >> Speed;
    _worldPacket >> FirePos;
    _worldPacket >> ImpactPos;
    bool hasStatus = _worldPacket.ReadBit();
    _worldPacket.ResetBitPos();
    if (hasStatus)
    {
        Status = boost::in_place();
        _worldPacket >> *Status;
    }
}

WorldPacket const* WorldPackets::Spells::SpellPrepare::Write()
{
    _worldPacket << ClientCastID;
    _worldPacket << ServerCastID;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Spells::PushSpellToActionBar::Write()
{
    _worldPacket << SpellID;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Spells::SpellFailureMessage::Write()
{
    _worldPacket << SpellID;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Spells::SpecializationChanged::Write()
{
    _worldPacket << SpecID;

    return &_worldPacket;
}

void WorldPackets::Spells::CancelModSpeedNoControlAuras::Read()
{
    _worldPacket >> TargetGUID;
}

void WorldPackets::Spells::UpdateSpellVisual::Read()
{
    _worldPacket >> SpellID;
    _worldPacket >> SpellXSpellVisualId;
    _worldPacket >> TargetGUID;
}
