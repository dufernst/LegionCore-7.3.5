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

#include "PartyPackets.h"

#include "Player.h"
#include "PlayerDefines.h"
#include "Pet.h"
#include "Vehicle.h"
#include "SpellAuras.h"
#include "SpellAuraEffects.h"
#include "ObjectMgr.h"

WorldPacket const* WorldPackets::Party::PartyCommandResult::Write()
{
    _worldPacket.WriteBits(Name.size(), 9);

    _worldPacket.WriteBits(Command, 4);
    _worldPacket.WriteBits(Result, 6);

    _worldPacket << ResultData;
    _worldPacket << ResultGUID;
    _worldPacket.WriteString(Name);

    return &_worldPacket;
}

void WorldPackets::Party::PartyInviteClient::Read()
{
    _worldPacket >> PartyIndex;
    _worldPacket >> ProposedRoles;
    _worldPacket >> TargetGUID;

    uint32 const targetNameLen = _worldPacket.ReadBits(9);
    uint32 const targetRealmLen = _worldPacket.ReadBits(9);

    TargetName = _worldPacket.ReadString(targetNameLen);
    TargetRealm = _worldPacket.ReadString(targetRealmLen);
}

WorldPacket const* WorldPackets::Party::PartyInvite::Write()
{
    // Order guessed
    _worldPacket.WriteBit(CanAccept);
    _worldPacket.WriteBit(MightCRZYou);
    _worldPacket.WriteBit(IsXRealm);
    _worldPacket.WriteBit(MustBeBNetFriend);
    _worldPacket.WriteBit(AllowMultipleRoles);

    _worldPacket.WriteBits(InviterName.size(), 6);

    _worldPacket << InviterVirtualRealmAddress;
    _worldPacket.WriteBit(IsLocal);
    _worldPacket.WriteBit(Unk2);
    _worldPacket.WriteBits(InviterRealmNameActual.size(), 8);
    _worldPacket.WriteBits(InviterRealmNameNormalized.size(), 8);
    _worldPacket.WriteString(InviterRealmNameActual);
    _worldPacket.WriteString(InviterRealmNameNormalized);

    _worldPacket << InviterGUID;
    _worldPacket << InviterBNetAccountId;

    _worldPacket << Unk1;
    _worldPacket << ProposedRoles;
    _worldPacket << static_cast<int32>(LfgSlots.size());
    _worldPacket << LfgCompletedMask;

    _worldPacket.WriteString(InviterName);

    for (int32 const& slot : LfgSlots)
        _worldPacket << slot;

    return &_worldPacket;
}

void WorldPackets::Party::PartyInvite::Initialize(Player* const inviter, int32 proposedRoles, bool canAccept)
{
    CanAccept = canAccept;

    InviterName = inviter->GetName();
    InviterGUID = inviter->GetGUID();
    InviterBNetAccountId = inviter->GetSession()->GetAccountGUID();

    ProposedRoles = proposedRoles;

    InviterVirtualRealmAddress = GetVirtualRealmAddress();
    InviterRealmNameActual = sObjectMgr->GetRealmName(realm.Id.Realm);
    InviterRealmNameNormalized = sObjectMgr->GetNormalizedRealmName(realm.Id.Realm);
}

void WorldPackets::Party::PartyInviteResponse::Read()
{
    _worldPacket >> PartyIndex;
    Accept = _worldPacket.ReadBit();
    if (_worldPacket.ReadBit())
    {
        RolesDesired = boost::in_place();
        _worldPacket >> *RolesDesired;
    }
}

void WorldPackets::Party::PartyUninvite::Read()
{
    _worldPacket >> PartyIndex;
    _worldPacket >> TargetGUID;
    Reason = _worldPacket.ReadString(_worldPacket.ReadBits(8));
}

WorldPacket const* WorldPackets::Party::GroupDecline::Write()
{
    _worldPacket.WriteBits(Name.length(), 9);
    _worldPacket.WriteString(Name);

    return &_worldPacket;
}

void WorldPackets::Party::RequestPartyMemberStats::Read()
{
    _worldPacket >> PartyIndex;
    _worldPacket >> TargetGUID;
}

WorldPacket const* WorldPackets::Party::PartyMemberStats::Write()
{
    _worldPacket.WriteBit(ForEnemy);
    _worldPacket.FlushBits();

    _worldPacket << MemberStats;

    return &_worldPacket;
}

void WorldPackets::Party::PartyMemberStatseUpdate::Initialize(Player* player)
{
    uint32 mask = player->GetGroupUpdateFlag();
    if (mask == GROUP_UPDATE_FLAG_NONE)
        return;

    if (mask & GROUP_UPDATE_FLAG_POWER_TYPE)
        mask |= GROUP_UPDATE_FLAG_CUR_POWER | GROUP_UPDATE_FLAG_MAX_POWER;

    ForEnemy = false;
    FullUpdate = false;
    MemberState.GUID = player->GetGUID();

    if (mask & GROUP_UPDATE_FLAG_STATUS)
    {
        MemberState.Status = boost::in_place();
        int16 memberStatus = MEMBER_STATUS_ONLINE;

        if (player->IsPvP())
            memberStatus |= MEMBER_STATUS_PVP;

        if (!player->isAlive())
        {
            if (player->HasFlag(PLAYER_FIELD_PLAYER_FLAGS, PLAYER_FLAGS_GHOST))
                memberStatus |= MEMBER_STATUS_GHOST;
            else
                memberStatus |= MEMBER_STATUS_DEAD;
        }

        if (player->IsFFAPvP())
            memberStatus |= MEMBER_STATUS_PVP_FFA;

        if (player->isAFK())
            memberStatus |= MEMBER_STATUS_AFK;

        if (player->isDND())
            memberStatus |= MEMBER_STATUS_DND;

        if (player->GetVehicle()) //! @TODO
            memberStatus |= MEMBER_STATUS_USING_VEHICLE;

        MemberState.Status = memberStatus;
    }

    if (mask & GROUP_UPDATE_FLAG_LEVEL)
    {
        MemberState.Level = boost::in_place();
        MemberState.Level = player->getLevel();
    }

    if (mask & GROUP_UPDATE_FLAG_CUR_HP)
    {
        MemberState.CurrentHealth = boost::in_place();
        MemberState.CurrentHealth = player->GetHealth();
    }

    if (mask & GROUP_UPDATE_FLAG_MAX_HP)
    {
        MemberState.MaxHealth = boost::in_place();
        MemberState.MaxHealth = player->GetMaxHealth();
    }

    if (mask & GROUP_UPDATE_FLAG_POWER_TYPE)
    {
        MemberState.PowerType = boost::in_place();
        MemberState.PowerType = player->getPowerType();
    }

    if (mask & GROUP_UPDATE_FLAG_CUR_POWER)
    {
        MemberState.CurrentPower = boost::in_place();
        MemberState.CurrentPower = player->GetPower(player->getPowerType());
    }

    if (mask & GROUP_UPDATE_FLAG_MAX_POWER)
    {
        MemberState.MaxPower = boost::in_place();
        MemberState.MaxPower = player->GetMaxPower(player->getPowerType());
    }

    if (mask & GROUP_UPDATE_FLAG_ZONE)
    {
        MemberState.ZoneID = boost::in_place();
        MemberState.ZoneID = player->GetCurrentZoneID();
    }

    if (mask & GROUP_UPDATE_FLAG_POSITION)
    {
        MemberState.Position = boost::in_place();
        MemberState.Position->PositionX = int16(player->GetPositionX());
        MemberState.Position->PositionY = int16(player->GetPositionY());
        MemberState.Position->PositionZ = int16(player->GetPositionZ());
    }

    if (mask & GROUP_UPDATE_FLAG_POWER_DISPLAY_ID)
    {
        MemberState.PowerDisplayID = boost::in_place();
        MemberState.PowerDisplayID = 0;
    }

    if (mask & GROUP_UPDATE_FLAG_OTHER_PARTY)
    {
        MemberState.PartyType[0] = boost::in_place();
        MemberState.PartyType[1] = boost::in_place();
        MemberState.PartyType[0] = player->GetByteValue(PLAYER_FIELD_BYTES_3, PLAYER_BYTES_3_OFFSET_PARTY_TYPE) & 0xF;
        MemberState.PartyType[1] = player->GetByteValue(PLAYER_FIELD_BYTES_3, PLAYER_BYTES_3_OFFSET_PARTY_TYPE) >> 4;
    }

    if (mask & GROUP_UPDATE_FLAG_WMO_GROUP_ID)
    {
        MemberState.WmoGroupID = boost::in_place();
        MemberState.WmoGroupID = 0;
    }

    if (mask & GROUP_UPDATE_FLAG_WMO_DOODAD_PLACEMENT_ID)
    {
        MemberState.WmoDoodadPlacementID = boost::in_place();
        MemberState.WmoDoodadPlacementID = 0;
    }

    if (mask & GROUP_UPDATE_FLAG_SPECIALIZATION_ID)
    {
        MemberState.SpecializationID = boost::in_place();
        MemberState.SpecializationID = player->GetUInt32Value(PLAYER_FIELD_CURRENT_SPEC_ID);
    }

    if (mask & GROUP_UPDATE_FLAG_VEHICLE_SEAT) //! @TODO
    {
        MemberState.VehicleSeat = boost::in_place();
        uint8 VehicleSeat = 0;
        if (auto const veh = player->GetVehicle())
            if (auto const vehInfo = veh->GetVehicleInfo())
                VehicleSeat = vehInfo->SeatID[player->m_movementInfo.transport.VehicleSeatIndex];

        MemberState.VehicleSeat = VehicleSeat;
    }

    if (mask & GROUP_UPDATE_FLAG_AURAS)
    {
        MemberState.AuraList = boost::in_place();
        Unit::VisibleAuraContainer const visibleAuras = player->GetVisibleAuras();
        for (AuraApplication const* aurApp : visibleAuras)
        {
            if (!aurApp || !aurApp->GetBase())
                continue;
            GroupAura aura;

            aura.SpellId = aurApp->GetBase()->GetId();
            aura.EffectMask = aurApp->GetEffectMask();
            aura.Scalings = aurApp->GetFlags(); // ??

            if (aurApp->GetFlags() & AFLAG_SCALABLE)
            {
                for (uint32 e = 0; e < MAX_SPELL_EFFECTS; ++e)
                {
                    float scale = 0.0f;
                    if (AuraEffect const* eff = aurApp->GetBase()->GetEffect(e))
                        scale = eff->GetAmount();
                    aura.EffectScales.push_back(scale);
                }
            }

            MemberState.AuraList->push_back(aura);
        }
    }

    if (mask & GROUP_UPDATE_FLAG_PHASE) //! @TODO
    {
        MemberState.Phases = boost::in_place();
        std::set<uint32> phases = player->GetPhases();
        MemberState.Phases->PhaseShiftFlags = 0x08 | (!phases.empty() ? 0x10 : 0);
        MemberState.Phases->PersonalGUID = ObjectGuid::Empty;
        for (uint32 phaseId : phases)
        {
            GroupPhase phase;
            phase.Id = phaseId;
            phase.Flags = 1;
            MemberState.Phases->List.push_back(phase);
        }
    }

    if (mask & GROUP_UPDATE_FLAG_PET)
    {
        if (Pet* pet = player->GetPet())
        {
            uint32 const petMask = pet->GetGroupUpdateFlag();
            if (petMask == GROUP_UPDATE_FLAG_PET_NONE)
                return;

            MemberState.PetStats = boost::in_place();

            if (petMask & GROUP_UPDATE_FLAG_PET_GUID) //! @TODO
                MemberState.PetStats->GUID = pet->GetGUID();

            if (petMask & GROUP_UPDATE_FLAG_PET_NAME)
                MemberState.PetStats->Name = pet->GetName();

            if (petMask & GROUP_UPDATE_FLAG_PET_MODEL_ID)
                MemberState.PetStats->ModelId = pet->GetDisplayId();

            if (petMask & GROUP_UPDATE_FLAG_PET_CUR_HP)
                MemberState.PetStats->CurrentHealth = pet->GetHealth();

            if (petMask & GROUP_UPDATE_FLAG_PET_MAX_HP)
                MemberState.PetStats->MaxHealth = pet->GetMaxHealth();

            if (petMask & GROUP_UPDATE_FLAG_PET_AURAS)
            {
                Unit::VisibleAuraContainer const visibleAuras = pet->GetVisibleAuras();
                for (AuraApplication const* aurApp : visibleAuras)
                {
                    if (!aurApp || !aurApp->GetBase())
                        continue;
                    GroupAura aura;

                    aura.SpellId = aurApp->GetBase()->GetId();
                    aura.EffectMask = aurApp->GetEffectMask();
                    aura.Scalings = aurApp->GetFlags(); // ??

                    if (aurApp->GetFlags() & AFLAG_SCALABLE)
                    {
                        for (uint32 e = 0; e < MAX_SPELL_EFFECTS; ++e)
                        {
                            float scale = 0.0f;
                            if (AuraEffect const* eff = aurApp->GetBase()->GetEffect(e))
                                scale = eff->GetAmount();
                            aura.EffectScales.push_back(scale);
                        }
                    }

                    MemberState.PetStats->AuraList.push_back(aura);
                }
            }
        }
    }
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Party::PartyMemberState const& memberState)
{
    data.WriteBit(memberState.PartyType[0].is_initialized() || memberState.PartyType[1].is_initialized());
    data.WriteBit(memberState.Status.is_initialized());
    data.WriteBit(memberState.PowerType.is_initialized());
    data.WriteBit(memberState.PowerDisplayID.is_initialized());
    data.WriteBit(memberState.CurrentHealth.is_initialized());
    data.WriteBit(memberState.MaxHealth.is_initialized());
    data.WriteBit(memberState.CurrentPower.is_initialized());
    data.WriteBit(memberState.MaxPower.is_initialized());
    data.WriteBit(memberState.Level.is_initialized());
    data.WriteBit(memberState.SpecializationID.is_initialized());
    data.WriteBit(memberState.ZoneID.is_initialized());
    data.WriteBit(memberState.WmoGroupID.is_initialized());
    data.WriteBit(memberState.WmoDoodadPlacementID.is_initialized());
    data.WriteBit(memberState.Position.is_initialized());
    data.WriteBit(memberState.VehicleSeat.is_initialized());
    data.WriteBit(memberState.AuraList.is_initialized());
    data.WriteBit(memberState.PetStats.is_initialized());
    data.WriteBit(memberState.Phases.is_initialized());
    data.FlushBits();

    if (memberState.PetStats)
        data << *memberState.PetStats;

    if (memberState.GUID)
        data << *memberState.GUID;

    if (memberState.PartyType[0].is_initialized() || memberState.PartyType[1].is_initialized())
        for (auto const& partyType : memberState.PartyType)
            data << *partyType;

    if (memberState.Status)
        data << *memberState.Status;

    if (memberState.PowerType)
        data << *memberState.PowerType;

    if (memberState.PowerDisplayID)
        data << *memberState.PowerDisplayID;

    if (memberState.CurrentHealth)
        data << *memberState.CurrentHealth;

    if (memberState.MaxHealth)
        data << *memberState.MaxHealth;

    if (memberState.CurrentPower)
        data << *memberState.CurrentPower;

    if (memberState.MaxPower)
        data << *memberState.MaxPower;

    if (memberState.Level)
        data << *memberState.Level;

    if (memberState.SpecializationID)
        data << *memberState.SpecializationID;

    if (memberState.ZoneID)
        data << *memberState.ZoneID;

    if (memberState.WmoGroupID)
        data << *memberState.WmoGroupID;

    if (memberState.WmoDoodadPlacementID)
        data << *memberState.WmoDoodadPlacementID;

    if (memberState.ZoneID)
        data << *memberState.ZoneID;

    if (memberState.Position)
    {
        data << memberState.Position->PositionX;
        data << memberState.Position->PositionY;
        data << memberState.Position->PositionZ;
    }

    if (memberState.VehicleSeat)
        data << *memberState.VehicleSeat;

    if (memberState.AuraList)
    {
        data << static_cast<int32>(memberState.AuraList->size());
        for (WorldPackets::Party::GroupAura const& aura : *memberState.AuraList)
            data << aura;
    }

    if (memberState.Phases)
        data << *memberState.Phases;

    return data;
}

WorldPacket const* WorldPackets::Party::PartyMemberStatseUpdate::Write()
{
    _worldPacket.WriteBit(ForEnemy);
    _worldPacket.WriteBit(FullUpdate);

    _worldPacket << MemberState;

    return &_worldPacket;
}

void WorldPackets::Party::SetPartyLeader::Read()
{
    _worldPacket >> PartyIndex;
    _worldPacket >> TargetGUID;
}

void WorldPackets::Party::SetRole::Read()
{
    _worldPacket >> PartyIndex;
    _worldPacket >> TargetGUID;
    _worldPacket >> Role;
}

WorldPacket const* WorldPackets::Party::RoleChangedInform::Write()
{
    _worldPacket << PartyIndex;
    _worldPacket << From;
    _worldPacket << ChangedUnit;
    _worldPacket << OldRole;
    _worldPacket << NewRole;

    return &_worldPacket;
}

void WorldPackets::Party::LeaveGroup::Read()
{
    _worldPacket >> PartyIndex;
}

void WorldPackets::Party::SetLootMethod::Read()
{
    _worldPacket >> PartyIndex;
    _worldPacket >> LootMethod;
    _worldPacket >> LootMasterGUID;
    _worldPacket >> LootThreshold;
}

void WorldPackets::Party::MinimapPingClient::Read()
{
    _worldPacket >> PositionX;
    _worldPacket >> PositionY;
    _worldPacket >> PartyIndex;
}

WorldPacket const* WorldPackets::Party::MinimapPing::Write()
{
    _worldPacket << Sender;
    _worldPacket << PositionX;
    _worldPacket << PositionY;

    return &_worldPacket;
}

void WorldPackets::Party::UpdateRaidTarget::Read()
{
    _worldPacket >> PartyIndex;
    _worldPacket >> Target;
    _worldPacket >> Symbol;
}

WorldPacket const* WorldPackets::Party::SendRaidTargetUpdateSingle::Write()
{
    _worldPacket << PartyIndex;
    _worldPacket << Symbol;
    _worldPacket << Target;
    _worldPacket << ChangedBy;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Party::SendRaidTargetUpdateAll::Write()
{
    _worldPacket << PartyIndex;

    _worldPacket << static_cast<int32>(TargetIcons.size());

    for (auto const& itr : TargetIcons)
    {
        _worldPacket << itr.second;
        _worldPacket << itr.first;
    }

    return &_worldPacket;
}

void WorldPackets::Party::ConvertRaid::Read()
{
    Raid = _worldPacket.ReadBit();
}

void WorldPackets::Party::RequestPartyJoinUpdates::Read()
{
    _worldPacket >> PartyIndex;
}

void WorldPackets::Party::SetAssistantLeader::Read()
{
    _worldPacket >> PartyIndex;
    _worldPacket >> Target;
    Apply = _worldPacket.ReadBit();
}

void WorldPackets::Party::DoReadyCheck::Read()
{
    _worldPacket >> PartyIndex;
}

WorldPacket const* WorldPackets::Party::ReadyCheckStarted::Write()
{
    _worldPacket << PartyIndex;
    _worldPacket << PartyGUID;
    _worldPacket << InitiatorGUID;
    _worldPacket << Duration;

    return &_worldPacket;
}

void WorldPackets::Party::ReadyCheckResponseClient::Read()
{
    _worldPacket >> PartyIndex;
    IsReady = _worldPacket.ReadBit();
}

WorldPacket const* WorldPackets::Party::ReadyCheckResponse::Write()
{
    _worldPacket << PartyGUID;
    _worldPacket << Player;
    _worldPacket.WriteBit(IsReady);
    _worldPacket.FlushBits();

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Party::ReadyCheckCompleted::Write()
{
    _worldPacket << PartyIndex;
    _worldPacket << PartyGUID;

    return &_worldPacket;
}

void WorldPackets::Party::OptOutOfLoot::Read()
{
    PassOnLoot = _worldPacket.ReadBit();
}

void WorldPackets::Party::InitiateRolePoll::Read()
{
    _worldPacket >> PartyIndex;
}

WorldPacket const* WorldPackets::Party::RolePollInform::Write()
{
    _worldPacket << PartyIndex;
    _worldPacket << From;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Party::GroupNewLeader::Write()
{
    _worldPacket << PartyIndex;
    _worldPacket.WriteBits(Name.size(), 6);
    _worldPacket.WriteString(Name);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Party::PartyUpdate::Write()
{
    _worldPacket << PartyFlags;
    _worldPacket << PartyIndex;
    _worldPacket << PartyType;

    _worldPacket << MyIndex;
    _worldPacket << PartyGUID;
    _worldPacket << SequenceNum;
    _worldPacket << LeaderGUID;

    _worldPacket << static_cast<int32>(PlayerList.size());

    _worldPacket.WriteBit(LfgInfos.is_initialized());
    _worldPacket.WriteBit(LootSettings.is_initialized());
    _worldPacket.WriteBit(DifficultySettings.is_initialized());
    _worldPacket.FlushBits();

    for (GroupPlayerInfos const& playerInfos : PlayerList)
        _worldPacket << playerInfos;

    if (LootSettings)
        _worldPacket << *LootSettings;

    if (DifficultySettings)
        _worldPacket << *DifficultySettings;

    if (LfgInfos)
        _worldPacket << *LfgInfos;

    return &_worldPacket;
}

void WorldPackets::Party::SetEveryoneIsAssistant::Read()
{
    _worldPacket >> PartyIndex;
    EveryoneIsAssistant = _worldPacket.ReadBit();
}

void WorldPackets::Party::ChangeSubGroup::Read()
{
    _worldPacket >> TargetGUID;
    _worldPacket >> PartyIndex;
    _worldPacket >> NewSubGroup;
}

void WorldPackets::Party::SwapSubGroups::Read()
{
    _worldPacket >> PartyIndex;
    _worldPacket >> FirstTarget;
    _worldPacket >> SecondTarget;
}

void WorldPackets::Party::ClearRaidMarker::Read()
{
    _worldPacket >> MarkerId;
}

WorldPacket const* WorldPackets::Party::RaidMarkersChanged::Write()
{
    _worldPacket << PartyIndex;
    _worldPacket << ActiveMarkers;

    _worldPacket.WriteBits(RaidMarkers.size(), 4);
    _worldPacket.FlushBits();

    for (RaidMarker* raidMarker : RaidMarkers)
    {
        _worldPacket << raidMarker->TransportGUID;
        _worldPacket << raidMarker->Location.GetMapId();
        _worldPacket << raidMarker->Location.PositionXYZStream();
    }

    return &_worldPacket;
}

void WorldPackets::Party::PartyMemberStats::Initialize(Player* player)
{
    ForEnemy = false;

    MemberStats.GUID = player->GetGUID();

    // Status
    MemberStats.Status = MEMBER_STATUS_ONLINE;

    if (player->IsPvP())
        MemberStats.Status |= MEMBER_STATUS_PVP;

    if (!player->isAlive())
    {
        if (player->HasFlag(PLAYER_FIELD_PLAYER_FLAGS, PLAYER_FLAGS_GHOST))
            MemberStats.Status |= MEMBER_STATUS_GHOST;
        else
            MemberStats.Status |= MEMBER_STATUS_DEAD;
    }

    if (player->IsFFAPvP())
        MemberStats.Status |= MEMBER_STATUS_PVP_FFA;

    if (player->isAFK())
        MemberStats.Status |= MEMBER_STATUS_AFK;

    if (player->isDND())
        MemberStats.Status |= MEMBER_STATUS_DND;

    if (player->GetVehicle())
        MemberStats.Status |= MEMBER_STATUS_USING_VEHICLE;

    // Level
    MemberStats.Level = player->getLevel();

    // Health
    MemberStats.CurrentHealth = player->GetHealth();
    MemberStats.MaxHealth = player->GetMaxHealth();

    // Power
    MemberStats.PowerType = player->getPowerType();
    MemberStats.CurrentPower = player->GetPower(player->getPowerType());
    MemberStats.MaxPower = player->GetMaxPower(player->getPowerType());

    // Position
    MemberStats.ZoneID = player->GetCurrentZoneID();
    MemberStats.PositionX = int16(player->GetPositionX());
    MemberStats.PositionY = int16(player->GetPositionY());
    MemberStats.PositionZ = int16(player->GetPositionZ());

    MemberStats.PowerDisplayID = 0;
    MemberStats.PartyType[0] = player->GetPartyTypeValue() & 0xF;
    MemberStats.PartyType[1] = player->GetPartyTypeValue() >> 4;
    MemberStats.WmoGroupID = 0;

    MemberStats.SpecializationID = player->GetUInt32Value(PLAYER_FIELD_CURRENT_SPEC_ID);
    MemberStats.WmoDoodadPlacementID = 0;

    // Vehicle
    if (auto const veh = player->GetVehicle())
        if (auto const vehInfo = veh->GetVehicleInfo())
            if (player->m_movementInfo.transport.VehicleSeatIndex <= 8 && player->m_movementInfo.transport.VehicleSeatIndex >= 0)
                MemberStats.VehicleSeat = vehInfo->SeatID[player->m_movementInfo.transport.VehicleSeatIndex];

    // Auras
    Unit::VisibleAuraContainer const visibleAuras = player->GetVisibleAuras();
    for (AuraApplication const* aurApp : visibleAuras)
    {
        if (!aurApp || !aurApp->GetBase())
            continue;
        GroupAura aura;

        aura.SpellId = aurApp->GetBase()->GetId();
        aura.EffectMask = aurApp->GetEffectMask();
        aura.Scalings = aurApp->GetFlags(); // ??

        if (aurApp->GetFlags() & AFLAG_SCALABLE)
        {
            for (uint32 e = 0; e < MAX_SPELL_EFFECTS; ++e)
            {
                float scale = 0.0f;
                if (AuraEffect const* eff = aurApp->GetBase()->GetEffect(e))
                    scale = eff->GetAmount();
                aura.EffectScales.push_back(scale);
            }
        }

        MemberStats.AuraList.push_back(aura);
    }

    // Phases
    std::set<uint32> phases = player->GetPhases();
    MemberStats.Phases.PhaseShiftFlags = 0x08 | (!phases.empty() ? 0x10 : 0);
    MemberStats.Phases.PersonalGUID = ObjectGuid::Empty;
    for (uint32 phaseId : phases)
    {
        GroupPhase phase;
        phase.Id = phaseId;
        phase.Flags = 1;
        MemberStats.Phases.List.push_back(phase);
    }

    // Pet
    if (player->GetPet())
    {
        Pet* pet = player->GetPet();

        MemberStats.PetStats = boost::in_place();

        MemberStats.PetStats->GUID = pet->GetGUID();
        MemberStats.PetStats->Name = pet->GetName();
        MemberStats.PetStats->ModelId = pet->GetDisplayId();

        MemberStats.PetStats->CurrentHealth = pet->GetHealth();
        MemberStats.PetStats->MaxHealth = pet->GetMaxHealth();

        Unit::VisibleAuraContainer const visibleAuras = pet->GetVisibleAuras();
        for (AuraApplication const* aurApp : visibleAuras)
        {
            if (!aurApp || !aurApp->GetBase())
                continue;
            GroupAura aura;

            aura.SpellId = aurApp->GetBase()->GetId();
            aura.EffectMask = aurApp->GetEffectMask();
            aura.Scalings = aurApp->GetFlags(); // ??

            if (aurApp->GetFlags() & AFLAG_SCALABLE)
            {
                for (uint32 e = 0; e < MAX_SPELL_EFFECTS; ++e)
                {
                    float scale = 0.0f;
                    if (AuraEffect const* eff = aurApp->GetBase()->GetEffect(e))
                        scale = eff->GetAmount();
                    aura.EffectScales.push_back(scale);
                }
            }

            MemberStats.PetStats->AuraList.push_back(aura);
        }
    }
}

void WorldPackets::Party::PartyMemberStats::Initialize(ObjectGuid guid)
{
    ForEnemy = false;

    MemberStats.GUID = guid;

    CharacterInfo const* characterInfo = sWorld->GetCharacterInfo(guid);

    // Status
    MemberStats.Status = MEMBER_STATUS_ONLINE;

    // Level
    MemberStats.Level = characterInfo ? characterInfo->Level : 110;

    // Health
    MemberStats.CurrentHealth = 100000;
    MemberStats.MaxHealth = 100000;

    // Power
    MemberStats.PowerType = POWER_MANA;
    MemberStats.CurrentPower = 0;
    MemberStats.MaxPower = 0;

    // Position
    MemberStats.ZoneID = 5841;
    MemberStats.PositionX = int16(4359.94f);
    MemberStats.PositionY = int16(2764.18f);
    MemberStats.PositionZ = int16(84.1021f);

    MemberStats.PowerDisplayID = 0;
    MemberStats.PartyType[0] = 0;
    MemberStats.PartyType[1] = 0;
    MemberStats.WmoGroupID = 0;

    MemberStats.SpecializationID = characterInfo ? characterInfo->SpecId : 0;
    MemberStats.WmoDoodadPlacementID = 0;

    // Phases
    MemberStats.Phases.PhaseShiftFlags = 0x08;
    MemberStats.Phases.PersonalGUID = ObjectGuid::Empty;
}

WorldPacket const* WorldPackets::Party::PartyKillLog::Write()
{
    _worldPacket << Player;
    _worldPacket << Victim;

    return &_worldPacket;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Party::GroupPhase const& phase)
{
    data << phase.Flags;
    data << phase.Id;

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Party::GroupPhases const& phases)
{
    data << phases.PhaseShiftFlags;
    data << static_cast<int32>(phases.List.size());
    data << phases.PersonalGUID;

    for (WorldPackets::Party::GroupPhase const& phase : phases.List)
        data << phase;

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Party::GroupAura const& aura)
{
    data << aura.SpellId;
    data << aura.Scalings;
    data << aura.EffectMask;

    data << int32(aura.EffectScales.size());
    for (float scale : aura.EffectScales)
        data << scale;

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, std::vector<WorldPackets::Party::GroupAura> const& auraList)
{
    data << int32(auraList.size());
    for (WorldPackets::Party::GroupAura const& aura : auraList)
        data << aura;

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Party::GroupPetStats const& petStats)
{
    data << petStats.GUID;

    data << petStats.ModelId;

    data << petStats.CurrentHealth;
    data << petStats.MaxHealth;

    data << petStats.AuraList;

    data.WriteBits(petStats.Name.size(), 8);
    data.WriteString(petStats.Name);

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Party::GroupMemberStats const& memberStats)
{
    for (auto const& partyType : memberStats.PartyType)
        data << partyType;

    data << memberStats.Status;

    data << memberStats.PowerType;

    data << memberStats.PowerDisplayID;

    data << memberStats.CurrentHealth;
    data << memberStats.MaxHealth;

    data << memberStats.CurrentPower;
    data << memberStats.MaxPower;

    data << memberStats.Level;

    data << memberStats.SpecializationID;

    data << memberStats.ZoneID;

    data << memberStats.WmoGroupID;
    data << memberStats.WmoDoodadPlacementID;

    data << memberStats.PositionX;
    data << memberStats.PositionY;
    data << memberStats.PositionZ;

    data << memberStats.VehicleSeat;

    data << static_cast<int32>(memberStats.AuraList.size());

    data << memberStats.Phases;

    for (WorldPackets::Party::GroupAura const& aura : memberStats.AuraList)
        data << aura;

    data.WriteBit(memberStats.PetStats.is_initialized());
    data.FlushBits();

    if (memberStats.PetStats)
        data << *memberStats.PetStats;

    data << memberStats.GUID;

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, std::vector<WorldPackets::Party::GroupPlayerInfos> const& playerList)
{
    data << static_cast<int32>(playerList.size());

    for (WorldPackets::Party::GroupPlayerInfos const& playerInfos : playerList)
        data << playerInfos;

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Party::GroupPlayerInfos const& playerInfos)
{
    data.WriteBits(playerInfos.Name.size(), 6);
    data.WriteBit(playerInfos.FromSocialQueue);
    data.FlushBits();

    data << playerInfos.GUID;
    data << playerInfos.Status;
    data << playerInfos.Subgroup;
    data << playerInfos.Flags;
    data << playerInfos.RolesAssigned;
    data << playerInfos.Class;

    data.WriteString(playerInfos.Name);

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Party::GroupLfgInfos const& lfgInfos)
{
    data << lfgInfos.MyFlags;
    data << lfgInfos.Slot;
    data << lfgInfos.MyRandomSlot;
    data << lfgInfos.MyPartialClear;
    data << lfgInfos.MyGearDiff;
    data << lfgInfos.MyStrangerCount;
    data << lfgInfos.MyKickVoteCount;
    data << lfgInfos.BootCount;

    data.WriteBit(lfgInfos.Aborted);
    data.WriteBit(lfgInfos.MyFirstReward);
    data.FlushBits();

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Party::GroupLootSettings const& lootSettings)
{
    data << lootSettings.Method;
    data << lootSettings.LootMaster;
    data << lootSettings.Threshold;

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Party::GroupDifficultySettings const& difficultySettings)
{
    data << difficultySettings.DungeonDifficultyID;
    data << difficultySettings.RaidDifficultyID;
    data << difficultySettings.LegacyRaidDifficultyID;

    return data;
}

void WorldPackets::Party::SetPartyAssignment::Read()
{
    _worldPacket >> PartyIndex;
    _worldPacket >> Assignment;
    _worldPacket >> Target;
    Set = _worldPacket.ReadBit();
}

WorldPacket const* WorldPackets::Party::ModifyPartyRange::Write()
{
    _worldPacket << Range;

    return &_worldPacket;
}
