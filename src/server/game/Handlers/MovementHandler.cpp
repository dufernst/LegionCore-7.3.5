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

#define MOVEMENT_PACKET_TIME_DELAY 0
#include "Anticheat.h"
#include "ObjectMgr.h"
#include "WaypointMovementGenerator.h"
#include "Garrison.h"
#include "PlayerDefines.h"
#include "InstancePackets.h"
#include "Vehicle.h"

void WorldSession::HandleWorldPortResponse(WorldPackets::Movement::WorldPortResponse& /*packet*/)
{
    HandleWorldPortAck();
}

void WorldSession::HandleWorldPortAck()
{
    Player* player = GetPlayer();
    if (!player || PlayerLogout())
        return;

    if (!player->IsBeingTeleportedFar() || player->IsChangeMap())
        return;

    player->SetChangeMap(true);
    bool seamlessTeleport = player->IsBeingTeleportedSeamlessly();

    Map* teleport = GetPlayer()->GetTeleportMap();
    GetPlayer()->ResetTeleMap();

    player->SetSemaphoreTeleportFar(false);

    if (Unit* mover = player->m_mover)
        mover->ClearUnitState(UNIT_STATE_JUMPING);

    Map* oldMap = player->GetMap();
    WorldLocation const loc = player->GetTeleportDest();
    if (!MapManager::IsValidMapCoord(loc))
    {
        player->SetChangeMap(false);
        LogoutPlayer(false);
        return;
    }

    MapEntry const* mEntry = sMapStore.LookupEntry(loc.GetMapId());
    InstanceTemplate const* mInstance = sObjectMgr->GetInstanceTemplate(loc.GetMapId());

    if (!player->m_InstanceValid && (!mInstance && !mEntry->IsScenario()))
        player->m_InstanceValid = true;

    if (player->IsInWorld())
        oldMap->RemovePlayerFromMap(player, false);

    Map* newMap = teleport ? teleport : sMapMgr->CreateMap(loc.GetMapId(), player);
    if (!newMap || !newMap->CanEnter(player))
    {
        player->TeleportTo(player->m_homebindMapId, player->m_homebindX, player->m_homebindY, player->m_homebindZ, player->GetOrientation());
        player->SetChangeMap(false);
        return;
    }
    if (newMap && GetMap() != newMap)
    {
        //move us to destination phread for future perfom addto map by this function
        TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "WorldSession::HandleMoveWorldportAckOpcode Map %d could not be enter player %d with bed session code", loc.GetMapId(), player->GetGUIDLow());
        player->TeleportTo(loc, GetPlayer()->GetTeleportOptions());
        return;
    }
    player->Relocate(&loc);

    if (player->InBattleground())
    {
        if (!mEntry->IsBattlegroundOrArena())
        {
            if (player->GetBattleground())
                player->LeaveBattleground(false);

            player->SetBattlegroundId(0, MS::Battlegrounds::BattlegroundTypeId::None);
            player->SetBGTeam(0);
            if (player->GetTransport())
            {
                player->m_transport->RemovePassenger(player);
                player->m_transport = nullptr;
                player->m_movementInfo.transport.Reset();
            }
        }
    }

    player->ResetMap();
    player->SetMap(newMap);

    if (!seamlessTeleport)
        player->SendInitialPacketsBeforeAddToMap(false);

    if (!player->GetMap()->AddPlayerToMap(player, !seamlessTeleport))
    {
        player->ResetMap();
        player->SetMap(oldMap);
        player->SetChangeMap(false);
        player->TeleportTo(player->m_homebindMapId, player->m_homebindX, player->m_homebindY, player->m_homebindZ, player->GetOrientation());
        return;
    }

    if (player->InBattleground())
        if (mEntry->IsBattlegroundOrArena())
            if (Battleground* bg = player->GetBattleground())
                if (player->IsInvitedForBattlegroundInstance(player->GetBattlegroundId()))
                    bg->AddPlayer(player);

    if (seamlessTeleport)
    {
        //player->UpdateVisibilityForPlayer();
        if (Garrison* garrison = player->GetGarrison())
            garrison->SendRemoteInfo();
    }
    else
        player->UpdateVisibilityForPlayer();

    if (player->GetMotionMaster()->GetCurrentMovementGeneratorType() == FLIGHT_MOTION_TYPE)
    {
        if (!seamlessTeleport)
        {
            if (!player->InBattleground())
            {
                auto flight = dynamic_cast<FlightPathMovementGenerator*>(player->GetMotionMaster()->top());
                flight->Initialize(*player);
            }
            else
            {
                player->GetMotionMaster()->MovementExpired();
                player->CleanupAfterTaxiFlight();
            }
        }
    }

    if (Corpse* corpse = sObjectAccessor->GetCorpseForPlayerGUID(player->GetGUID()))
    {
        if (corpse->GetType() != CORPSE_BONES && corpse->GetMapId() == player->GetMapId())
        {
            if (mEntry->IsDungeon())
            {
                player->ResurrectPlayer(0.5f, false);
                player->SpawnCorpseBones();
            }
        }
    }

    bool allowMount = !mEntry->IsDungeon() || mEntry->IsBattlegroundOrArena();
    if (mInstance)
    {
        Difficulty diff = player->GetDifficultyID(mEntry);
        if (MapDifficultyEntry const* mapDiff = sDB2Manager.GetMapDifficultyData(mEntry->ID, diff))
            if (mapDiff->GetRaidDuration())
                if (time_t timeReset = sWorld->getInstanceResetTime(mapDiff->GetRaidDuration()))
                    player->SendInstanceResetWarning(mEntry->ID, diff, uint32(timeReset - time(nullptr)));

        allowMount = mInstance->AllowMount;
    }

    if (!allowMount || (player->GetMapId() == 530 && player->GetCurrentZoneID() == 0))
        player->RemoveAurasByType(SPELL_AURA_MOUNTED);

    if (player->pvpInfo.inHostileArea)
        player->CastSpell(player, 2479, true);
    else if (player->IsPvP() && !player->HasFlag(PLAYER_FIELD_PLAYER_FLAGS, PLAYER_FLAGS_IN_PVP))
        player->UpdatePvP(false, false);

    player->ResummonPetTemporaryUnSummonedIfAny();
    player->SummonLastSummonedBattlePet();

    player->ProcessDelayedOperations();
    player->ReCreateAreaTriggerObjects();
    player->SetLastWorldStateUpdateTime(time_t(0));
    player->SetChangeMap(false);
}

void WorldSession::HandleMoveTeleportAck(WorldPackets::Movement::MoveTeleportAck& packet)
{
    Player* plMover = _player->m_mover->ToPlayer();

    if (plMover && plMover->IsBeingTeleportedSeamlessly())
        return;

    if (!plMover || !plMover->IsBeingTeleportedNear())
        return;

    if (packet.MoverGUID != plMover->GetGUID())
        return;

    plMover->SetSemaphoreTeleportNear(false);

    uint32 old_zone = plMover->GetCurrentZoneID();
    plMover->UpdatePosition(plMover->GetTeleportDest(), true);

    uint32 newzone, newarea;
    plMover->GetZoneAndAreaId(newzone, newarea);
    plMover->UpdateZone(newzone, newarea);

    if (old_zone != newzone)
    {
        if (plMover->pvpInfo.inHostileArea)
            plMover->CastSpell(plMover, 2479, true);

        else if (plMover->IsPvP() && !plMover->HasFlag(PLAYER_FIELD_PLAYER_FLAGS, PLAYER_FLAGS_IN_PVP))
            plMover->UpdatePvP(false, false);
    }

    GetPlayer()->ResummonPetTemporaryUnSummonedIfAny();
    GetPlayer()->SummonLastSummonedBattlePet();
    GetPlayer()->ProcessDelayedOperations();
    GetPlayer()->ReCreateAreaTriggerObjects();

    if (Unit* mover = _player->m_mover)
    {
        mover->m_movementInfo.MoveTime = getMSTime();
        mover->m_movementInfo.ClientMoveTime = packet.ClientMoveTime;
        mover->m_movementInfo.Pos = mover->GetPosition();

        //WorldPackets::Movement::MoveUpdateTeleport packet;
        //packet.movementInfo = &mover->m_movementInfo;
        //packet.movementInfo->RemoteTimeValid = true;
        //mover->SendMessageToSet(packet.Write(), mover);

        mover->ClearUnitState(UNIT_STATE_JUMPING);
    }
}

void WorldSession::HandleMovementOpcodes(WorldPackets::Movement::ClientPlayerMovement& packet)
{
    HandleMovementOpcode(packet.GetOpcode(), packet.movementInfo);
}

void WorldSession::HandleMovementOpcode(OpcodeClient opcode, MovementInfo& movementInfo)
{
    Unit* mover = _player->m_mover;

    if (!mover)
        return;

    Player* plrMover = mover->ToPlayer();
    if (mover->GetVehicleKit())
        if (mover->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED))
            if (Unit* charmer = mover->GetCharmer())
                if (charmer->IsPlayer())
                    plrMover = charmer->ToPlayer();

    if (plrMover && plrMover->IsBeingTeleported())
        return;

    GetPlayer()->ValidateMovementInfo(&movementInfo);
    if (movementInfo.Guid != mover->GetGUID() || !movementInfo.Pos.IsPositionValid())
        return;

    if (plrMover && !plrMover->GetCheatData()->HandleAnticheatTests(movementInfo, this, opcode))
        return;

    if (plrMover && (plrMover->GetUInt32Value(UNIT_FIELD_EMOTE_STATE) != 0))
        plrMover->SetUInt32Value(UNIT_FIELD_EMOTE_STATE, EMOTE_ONESHOT_NONE);

    if (mover->HasAuraType(SPELL_AURA_MOD_POSSESS) || (plrMover && plrMover->HasAuraType(SPELL_AURA_MOD_POSSESS)))
        if (movementInfo.HasMovementFlag(MOVEMENTFLAG_WALKING))
            movementInfo.RemoveMovementFlag(MOVEMENTFLAG_WALKING);

    RelocateMover(movementInfo);

    // handle fall on server
    // fall damage generation (ignore in flight case that can be triggered also at lags in moment teleportation to another map).
    if (opcode == CMSG_MOVE_FALL_LAND && plrMover && !plrMover->isInFlight())
    {
        plrMover->HandleFall(movementInfo);
        plrMover->ClearUnitState(UNIT_STATE_LONG_JUMP);

        if (plrMover->GetKnockBackTime())
        {
            plrMover->SetKnockBackTime(0);
            plrMover->ClearUnitState(UNIT_STATE_JUMPING);
        }
    }

    if (plrMover)
    {
        if (plrMover->IsSitState() && (movementInfo.HasMovementFlag(MOVEMENTFLAG_MASK_MOVING | MOVEMENTFLAG_MASK_TURNING)))
            plrMover->SetStandState(UNIT_STAND_STATE_STAND);

        plrMover->UpdateFallInformationIfNeed(movementInfo, opcode);

        if (movementInfo.Pos.GetPositionZ() < plrMover->GetMap()->GetMinHeight(movementInfo.Pos))
        {
            if (!(plrMover->GetBattleground() && plrMover->GetBattleground()->HandlePlayerUnderMap(_player)))
            {
                if (plrMover->isAlive())
                {
                    //! Client Crash.
                    //plrMover->SetFlag(PLAYER_FIELD_PLAYER_FLAGS, PLAYER_FLAGS_IS_OUT_OF_BOUNDS);
                    plrMover->EnvironmentalDamage(DAMAGE_FALL_TO_VOID, GetPlayer()->GetMaxHealth());
                    if (!plrMover->isAlive())
                        plrMover->KillPlayer();
                }
            }
        }

        if (opcode == CMSG_MOVE_JUMP)
        {
            DamageInfo dmgInfoProc = DamageInfo(mover, mover, 0, nullptr, SPELL_SCHOOL_MASK_NORMAL, DIRECT_DAMAGE, 0);
            mover->ProcDamageAndSpellFor(false, mover, PROC_FLAG_ON_JUMP, PROC_EX_NORMAL_HIT, BASE_ATTACK, nullptr, &dmgInfoProc);
        
            plrMover->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_JUMP, 0, 605); // Mind Control
        }

        const bool no_fly_auras = !(plrMover->HasAuraType(SPELL_AURA_FLY) || plrMover->HasAuraType(SPELL_AURA_MOD_INCREASE_VEHICLE_FLIGHT_SPEED)
            || plrMover->HasAuraType(SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED) || plrMover->HasAuraType(SPELL_AURA_MOD_INCREASE_FLIGHT_SPEED)
            || plrMover->HasAuraType(SPELL_AURA_MOD_MOUNTED_FLIGHT_SPEED_ALWAYS));
        const bool fly_flags = movementInfo.HasMovementFlag(MOVEMENTFLAG_CAN_FLY | MOVEMENTFLAG_FLYING | MOVEMENTFLAG_DISABLE_GRAVITY);

        if (plrMover->GetSession()->GetSecurity() < SEC_MODERATOR)
        {
            if (no_fly_auras && fly_flags)
                plrMover->SetCanFly(false);

            if (!plrMover->HasAuraType(SPELL_AURA_DISABLE_GRAVITY) && movementInfo.HasMovementFlag(MOVEMENTFLAG_DISABLE_GRAVITY))
                plrMover->SetDisableGravity(false);
        }
    }

    // update change mover position to other clients
    WorldPackets::Movement::MoveUpdate moveUpdate;
    moveUpdate.movementInfo = &mover->m_movementInfo;
    moveUpdate.movementInfo->RemoteTimeValid = true;
    mover->SendMessageToSet(moveUpdate.Write(), _player);
}

void WorldSession::HandleForceSpeedChangeAck(WorldPackets::Movement::MovementSpeedAck& packet)
{
    Player* player = GetPlayer();
    player->ValidateMovementInfo(&packet.Ack.movementInfo);

    if (player->GetGUID() != packet.Ack.movementInfo.Guid)
        return;

    // Process anticheat checks, remember client-side speed ...
    if (_player->m_mover == _player && !_player->GetCheatData()->HandleSpeedChangeAck(packet.Ack.movementInfo, this, packet.GetOpcode(), packet.Speed))
        return;

    // relocate
    RelocateMover(packet.Ack.movementInfo);
    ///@ Kills a player after resurrection!
    //player->UpdateFallInformationIfNeed(packet.Ack.movementInfo, packet.GetOpcode());

    UnitMoveType moveType;
    OpcodeServer updateOpcode;
    switch (packet.GetOpcode())
    {
        case CMSG_MOVE_FORCE_WALK_SPEED_CHANGE_ACK:        moveType = MOVE_WALK;        updateOpcode = SMSG_MOVE_UPDATE_WALK_SPEED;      break;
        case CMSG_MOVE_FORCE_RUN_SPEED_CHANGE_ACK:         moveType = MOVE_RUN;         updateOpcode = SMSG_MOVE_UPDATE_RUN_SPEED;       break;
        case CMSG_MOVE_FORCE_RUN_BACK_SPEED_CHANGE_ACK:    moveType = MOVE_RUN_BACK;    updateOpcode = SMSG_MOVE_UPDATE_RUN_BACK_SPEED;  break;
        case CMSG_MOVE_FORCE_SWIM_SPEED_CHANGE_ACK:        moveType = MOVE_SWIM;        updateOpcode = SMSG_MOVE_UPDATE_SWIM_SPEED;      break;
        case CMSG_MOVE_FORCE_SWIM_BACK_SPEED_CHANGE_ACK:   moveType = MOVE_SWIM_BACK;   updateOpcode = SMSG_MOVE_UPDATE_SWIM_BACK_SPEED; break;
        case CMSG_MOVE_FORCE_TURN_RATE_CHANGE_ACK:         moveType = MOVE_TURN_RATE;   updateOpcode = SMSG_MOVE_UPDATE_TURN_RATE;       break;
        case CMSG_MOVE_FORCE_FLIGHT_SPEED_CHANGE_ACK:      moveType = MOVE_FLIGHT;      updateOpcode = SMSG_MOVE_UPDATE_FLIGHT_SPEED;    break;
        case CMSG_MOVE_FORCE_FLIGHT_BACK_SPEED_CHANGE_ACK: moveType = MOVE_FLIGHT_BACK; updateOpcode = SMSG_MOVE_UPDATE_FLIGHT_BACK_SPEED;  break;
        case CMSG_MOVE_FORCE_PITCH_RATE_CHANGE_ACK:        moveType = MOVE_PITCH_RATE;  updateOpcode = SMSG_MOVE_UPDATE_PITCH_RATE;  break;
        default:
            return;
    }

    // Send notification to other players
    WorldPackets::Movement::MoveUpdateSpeed speedPacket(updateOpcode);
    speedPacket.movementInfo = &player->m_movementInfo;
    speedPacket.movementInfo->RemoteTimeValid = true;
    speedPacket.Speed = player->GetSpeed(moveType);
    player->SendMessageToSet(speedPacket.Write(), false);
}

void WorldSession::HandleMoveKnockBackAck(WorldPackets::Movement::MoveKnockBackAck& packet)
{
    Player* player = GetPlayer();
    Unit* mover = player->m_mover;
    if (!mover)
        return;

    player->ValidateMovementInfo(&packet.Ack.movementInfo);

    if (player->m_mover->GetGUID() != packet.Ack.movementInfo.Guid)
        return;

    // relocate
    RelocateMover(packet.Ack.movementInfo);
    player->UpdateFallInformationIfNeed(packet.Ack.movementInfo, packet.GetOpcode());

    //WorldPackets::Movement::MoveUpdateKnockBack updateKnockBack;
    //updateKnockBack.movementInfo = &player->m_movementInfo;
    //updateKnockBack.movementInfo->RemoteTimeValid = true;
    //player->SendMessageToSet(updateKnockBack.Write(), false);

    mover->ClearUnitState(UNIT_STATE_JUMPING);
}

void WorldSession::HandleMovementAckMessage(WorldPackets::Movement::MovementAckMessage& packet)
{
    Player* player = GetPlayer();

    player->ValidateMovementInfo(&packet.Ack.movementInfo);

    if (player->m_mover->GetGUID() != packet.Ack.movementInfo.Guid)
        return;

    if (!_player->GetCheatData()->HandleAnticheatTests(packet.Ack.movementInfo, this, packet.GetOpcode()))
        return;

    // relocate
    RelocateMover(packet.Ack.movementInfo);
    ///@ Kills a player after resurrection!
    //player->UpdateFallInformationIfNeed(packet.Ack.movementInfo, packet.GetOpcode());

    // maybe i use this switch later :)
    /*OpcodeServer updateOpcode;
    switch (packet.GetOpcode())
    {
        case CMSG_MOVE_ENABLE_DOUBLE_JUMP_ACK:             updateOpcode = SMSG_MOVE_UPDATE_WALK_SPEED;      break;
        case CMSG_MOVE_ENABLE_SWIM_TO_FLY_TRANS_ACK:       updateOpcode = SMSG_MOVE_UPDATE_RUN_SPEED;       break;
        case CMSG_MOVE_FEATHER_FALL_ACK:                   updateOpcode = SMSG_MOVE_UPDATE_RUN_BACK_SPEED;  break;
        case CMSG_MOVE_FORCE_ROOT_ACK:                     updateOpcode = SMSG_MOVE_UPDATE_SWIM_SPEED;      break;
        case CMSG_MOVE_FORCE_UNROOT_ACK:                   updateOpcode = SMSG_MOVE_UPDATE_SWIM_BACK_SPEED; break;
        case CMSG_MOVE_GRAVITY_DISABLE_ACK:                updateOpcode = SMSG_MOVE_UPDATE_TURN_RATE;       break;
        case CMSG_MOVE_GRAVITY_ENABLE_ACK:                 updateOpcode = SMSG_MOVE_UPDATE_FLIGHT_SPEED;    break;
        case CMSG_MOVE_HOVER_ACK:                          updateOpcode = SMSG_MOVE_UPDATE_FLIGHT_BACK_SPEED;  break;
        case CMSG_MOVE_SET_CAN_FLY_ACK:                    updateOpcode = SMSG_MOVE_UPDATE_PITCH_RATE;  break;
        case CMSG_MOVE_SET_CAN_TURN_WHILE_FALLING_ACK:     updateOpcode = SMSG_MOVE_UPDATE_FLIGHT_SPEED;    break;
        case CMSG_MOVE_SET_IGNORE_MOVEMENT_FORCES_ACK:     updateOpcode = SMSG_MOVE_UPDATE_FLIGHT_BACK_SPEED;  break;
        case CMSG_MOVE_WATER_WALK_ACK:                     updateOpcode = SMSG_MOVE_UPDATE_PITCH_RATE;  break;
        default:
            return;
    }*/

    // we don't have special opcodes for send movement flags to other clients, simply use SMSG_MOVE_UPDATE with correct moveflags
    WorldPackets::Movement::MoveUpdate moveUpdate;
    moveUpdate.movementInfo = &player->m_movementInfo;
    moveUpdate.movementInfo->RemoteTimeValid = true;
    player->SendMessageToSet(moveUpdate.Write(), false);
}

void WorldSession::HandleSetCollisionHeightAck(WorldPackets::Movement::MoveSetCollisionHeightAck& packet)
{
    Player* player = GetPlayer();
    player->ValidateMovementInfo(&packet.Data.movementInfo);
    if (player->m_mover->GetGUID() != packet.Data.movementInfo.Guid)
        return;

    // relocate
    RelocateMover(packet.Data.movementInfo);
    player->UpdateFallInformationIfNeed(packet.Data.movementInfo, packet.GetOpcode());

    WorldPackets::Movement::MoveUpdateCollisionHeight moveUpdate;
    moveUpdate.movementInfo = &player->m_movementInfo;
    moveUpdate.movementInfo->RemoteTimeValid = true;
    moveUpdate.Height = packet.MsgData.Height;
    moveUpdate.Scale = player->GetFloatValue(OBJECT_FIELD_SCALE);
    player->SendMessageToSet(moveUpdate.Write(), false);
}

void WorldSession::HandleSetActiveMover(WorldPackets::Movement::SetActiveMover& packet)
{
    if (Player* player = GetPlayer())
    {
        if (player->IsInWorld() && player->m_mover && player->m_mover->IsInWorld())
        {
            if (player->m_mover->GetGUID() != packet.ActiveMover)
                TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "HandleSetActiveMover: incorrect mover guid: mover is %s  and should be %s",
                    packet.ActiveMover.ToString().c_str(), player->m_mover->GetGUID().ToString().c_str());
        }
    }
}

void WorldSession::HandleMoveTimeSkipped(WorldPackets::Movement::MoveTimeSkipped& packet)
{
    Player* player = GetPlayer();
    player->m_movementInfo.MoveTime += packet.TimeSkipped;
    player->m_movementInfo.ClientMoveTime += packet.TimeSkipped;

    WorldPackets::Movement::MoveSkipTime lagSync;
    lagSync.MoverGUID = player->GetGUID();
    lagSync.SkippedTime = packet.TimeSkipped;
    player->SendMessageToSet(lagSync.Write(), false);
}

void WorldSession::HandleMoveSplineDone(WorldPackets::Movement::MoveSplineDone& packet)
{
    Player* player = GetPlayer();
    MovementInfo movementInfo = packet.movementInfo;
    _player->ValidateMovementInfo(&movementInfo);

    // in taxi flight packet received in 2 case:
    // 1) end taxi path in far (multi-node) flight
    // 2) switch from one map to other in case multim-map taxi path
    // we need process only (1)
    uint32 curDest = player->m_taxi.GetTaxiDestination();
    if (curDest)
    {
        TaxiNodesEntry const* curDestNode = sTaxiNodesStore.LookupEntry(curDest);

        // far teleport case
        if (curDestNode && curDestNode->ContinentID != player->GetMapId())
        {
            if (player->GetMotionMaster()->GetCurrentMovementGeneratorType() == FLIGHT_MOTION_TYPE)
            {
                auto flight = dynamic_cast<FlightPathMovementGenerator*>(player->GetMotionMaster()->top());

                flight->SetCurrentNodeAfterTeleport();
                TaxiPathNodeEntry const* node = flight->GetPath()[flight->GetCurrentNode()];
                flight->SkipCurrentNode();

                player->TeleportTo(curDestNode->ContinentID, node->Loc.X, node->Loc.Y, node->Loc.Z, player->GetOrientation());
            }
        }

        return;
    }

    if (player->m_taxi.GetPath().size() != 1)
    {
        // relocate
        RelocateMover(movementInfo);
        player->UpdateFallInformationIfNeed(movementInfo, packet.GetOpcode());
        return;
    }

    player->CleanupAfterTaxiFlight();
    player->SetFallInformation(0, player->GetPositionZ());
    if (player->pvpInfo.inHostileArea)
        player->CastSpell(player, 2479, true);
}

void WorldSession::HandleMoveRemoveMovementForceAck(WorldPackets::Movement::MoveRemoveMovementForceAck& packet)
{
    Player* player = GetPlayer();
    player->ValidateMovementInfo(&packet.Ack.movementInfo);
    if (player->m_mover->GetGUID() != packet.Ack.movementInfo.Guid)
        return;

    // relocate
    RelocateMover(packet.Ack.movementInfo);
    player->UpdateFallInformationIfNeed(packet.Ack.movementInfo, packet.GetOpcode());
}

void WorldSession::HandleMoveApplyMovementForceAck(WorldPackets::Movement::MoveApplyMovementForceAck& packet)
{
    Player* player = GetPlayer();
    player->ValidateMovementInfo(&packet.Ack.movementInfo);
    if (player->m_mover->GetGUID() != packet.Ack.movementInfo.Guid)
        return;

    // relocate
    RelocateMover(packet.Ack.movementInfo);
    player->UpdateFallInformationIfNeed(packet.Ack.movementInfo, packet.GetOpcode());
}

void WorldSession::HandleTimeSyncResponse(WorldPackets::Movement::TimeSyncResponse& packet)
{
    Player* player = GetPlayer();

    if (player->m_timeSyncQueue.empty())
    {
        TC_LOG_DEBUG(LOG_FILTER_NETWORKIO, "Received CMSG_TIME_SYNC_RESPONSE from player %s without requesting it (hacker?)", player->GetName());
        return;
    }

    if (packet.SequenceIndex != player->m_timeSyncQueue.front())
        TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "Wrong time sync counter from player %s (cheater?)", player->GetName());

    player->m_timeSyncClient = packet.ClientTime;
    player->m_timeSyncQueue.pop();
}

void WorldSession::HandleDiscardedTimeSyncAcks(WorldPackets::Movement::DiscardedTimeSyncAcks& packet)
{
    Player* player = GetPlayer();

    if (player->m_sequenceIndex != packet.MaxSequenceIndex)
        TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "Received CMSG_DISCARDED_TIME_SYNC_ACKS from player %s, but maxSequenceIndex %u isn't equal real server SequenceIndex %u", player->GetName(), packet.MaxSequenceIndex, player->m_sequenceIndex);

    player->m_sequenceIndex = 0;
}

void WorldSession::HandleTimeSyncResponseDropped(WorldPackets::Movement::TimeSyncResponseDropped& /*packet*/)
{
    //GetPlayer()->m_sequenceIndex = std::min(packet.SequenceIndexFirst, packet.SequenceIndexLast);
}

void WorldSession::HandleTimeSyncResponseFailed(WorldPackets::Movement::TimeSyncResponseFailed& packet)
{
    Player* player = GetPlayer();

    TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "WorldSession::HandleTimeSyncResponseFailed:: what we should do with it?: Player: %s, packet.SequenceIndex: %u, PlayerSequenceIndex: %u ",
        player->GetName(), packet.SequenceIndex, player->m_sequenceIndex);
}

void WorldSession::HandleSuspendTokenResponse(WorldPackets::Movement::SuspendTokenResponse& /*suspendTokenResponse*/)
{
    if (!_player->IsBeingTeleportedFar())
        return;

    bool seamlessTeleport = _player->IsBeingTeleportedSeamlessly();

    WorldLocation const& loc = GetPlayer()->GetTeleportDest();
    if (sMapStore.AssertEntry(loc.GetMapId())->IsDungeon())
        SendPacket(WorldPackets::Instance::UpdateLastInstance(loc.GetMapId()).Write());

    WorldPackets::Movement::NewWorldReason reason = WorldPackets::Movement::NewWorldReason::NORMAL;
    if (seamlessTeleport)
        reason = WorldPackets::Movement::NewWorldReason::SEAMLESS;
    else if (sMapStore.AssertEntry(loc.GetMapId())->IsBattlegroundOrArena())
        reason = WorldPackets::Movement::NewWorldReason::UNK_1;

    WorldPackets::Movement::NewWorld packet;
    packet.MapID = loc.GetMapId();
    packet.Pos = loc;
    packet.Reason = reason;
    SendPacket(packet.Write());

    WorldPackets::Movement::ResumeToken resumeToken;
    resumeToken.SequenceIndex = 0;
    resumeToken.Reason = seamlessTeleport ? 2 : 1;
    SendPacket(resumeToken.Write());

    if (seamlessTeleport)
        HandleWorldPortAck();
}

void WorldSession::RelocateMover(MovementInfo &movementInfo)
{
    Unit* mover = _player->m_mover;
    if (!mover)
        return;

    Map* map = mover->GetMap();
    Player* plrMover = mover->ToPlayer();
    Vehicle const* vehMover = mover->GetVehicleKit();

    if (vehMover)
    {
        if (mover->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED))
            if (Unit* charmer = mover->GetCharmer())
                if (charmer->IsPlayer())
                    plrMover = charmer->ToPlayer();

        // exit from vehicle before real relocation on server
        if (plrMover)
        {
            if (Unit* base = vehMover->GetBase())
                if (Creature* vehCreature = base->ToCreature())
                    if (!vehCreature->isInAccessiblePlaceFor(vehCreature))
                        plrMover->ExitVehicle();
        }
    }

    // handle transport boarded / unboarded passenger on server - some transport checks must be moved in anticheat manager
    if (!movementInfo.transport.Guid.IsEmpty())
    {
        if (movementInfo.transport.Pos.GetPositionX() > 75.0f || movementInfo.transport.Pos.GetPositionY() > 75.0f || movementInfo.transport.Pos.GetPositionZ() > 75.0f)
            return;

        if (!Trinity::IsValidMapCoord(movementInfo.Pos.GetPositionX() + movementInfo.transport.Pos.GetPositionX(), movementInfo.Pos.GetPositionY() + movementInfo.transport.Pos.GetPositionY(),
            movementInfo.Pos.GetPositionZ() + movementInfo.transport.Pos.GetPositionZ(), movementInfo.Pos.GetOrientation() + movementInfo.transport.Pos.GetOrientation()))
            return;

        if (plrMover)
        {
            if (!plrMover->GetTransport())
            {
                if (Transport* transport = map->GetTransport(movementInfo.transport.Guid))
                    transport->AddPassenger(plrMover);
                else if (StaticTransport* transport = map->GetStaticTransport(movementInfo.transport.Guid))
                    transport->AddPassenger(plrMover);
            }
            else if (plrMover->GetTransport()->GetGUID() != movementInfo.transport.Guid)
            {
                plrMover->GetTransport()->RemovePassenger(plrMover);
                if (Transport* transport = map->GetTransport(movementInfo.transport.Guid))
                    transport->AddPassenger(plrMover);
                else if (StaticTransport* transport = map->GetStaticTransport(movementInfo.transport.Guid))
                    transport->AddPassenger(plrMover);
                else if (!mover->GetVehicle())
                    movementInfo.ResetTransport();
            }
        }

        if (!mover->GetTransport() && !mover->GetVehicle())
            movementInfo.transport.Reset();
    }
    else if (plrMover)
    {
        if (Transport* transport = plrMover->GetTransport())
        {
            plrMover->m_transport = nullptr;
            transport->RemovePassenger(plrMover);
        }
    }

    // handle water - client send swimming flag, we need update state on server and some checks it
    if (plrMover)
    {
        if (movementInfo.HasMovementFlag(MOVEMENTFLAG_SWIMMING) != plrMover->IsInWater())
            plrMover->SetInWater(!plrMover->IsInWater() || map->IsUnderWater(G3D::Vector3(movementInfo.Pos.m_positionX, movementInfo.Pos.m_positionY, movementInfo.Pos.m_positionZ)));
    }

    // SERVER REAL RELOCATION

    // some vehicle checks
    if (Vehicle* vehicle = mover->GetVehicle())
    {
        if (VehicleSeatEntry const* seat = vehicle->GetSeatForPassenger(mover))
        {
            if (seat->Flags & VEHICLE_SEAT_FLAG_ALLOW_TURNING)
            {
                if (movementInfo.Pos.GetOrientation() != mover->GetOrientation())
                {
                    mover->SetOrientation(movementInfo.Pos.GetOrientation());
                    mover->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_TURNING);
                }
            }
        }

        movementInfo.Guid = mover->GetGUID();
        mover->m_movementInfo = movementInfo;
        return;
    }

    // update position
    mover->UpdatePosition(movementInfo.Pos);

    if (plrMover)                                            // nothing is charmed, or player charmed
    {
        if (movementInfo.HasMovementFlag(MOVEMENTFLAG_FALLING_FAR))
        {
            float hauteur = plrMover->GetMap()->GetHeight(plrMover->GetPositionX(), plrMover->GetPositionY(), plrMover->GetPositionZ(), true);
            bool undermap = false;
            // Undermap
            if ((plrMover->GetPositionZ() + 100.0f) < hauteur)
                undermap = true;
            if (plrMover->GetPositionZ() < 250.0f && plrMover->GetMapId() == 489)
                undermap = true;

            if (undermap)
                if (plrMover->UndermapRecall())
                    TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "[UNDERMAP] %s [GUID %u]. MapId:%u %f %f %f", plrMover->GetName(), plrMover->GetGUIDLow(), plrMover->GetMapId(), plrMover->GetPositionX(), plrMover->GetPositionY(), plrMover->GetPositionZ());
        }
        else if (plrMover->CanFreeMove())
            plrMover->SaveNoUndermapPosition(movementInfo.Pos.GetPositionX(), movementInfo.Pos.GetPositionY(), movementInfo.Pos.GetPositionZ() + 3.0f);
    }

    // update movement info
    movementInfo.Guid = mover->GetGUID();
    mover->m_movementInfo = movementInfo;
}
