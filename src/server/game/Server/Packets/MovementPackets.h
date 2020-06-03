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

#ifndef MovementPackets_h__
#define MovementPackets_h__

#include "Packet.h"
#include "Object.h"

namespace Movement
{
    class MoveSpline;
}

namespace WorldPackets
{
    namespace Movement
    {
        class ClientPlayerMovement final : public ClientPacket
        {
        public:
            ClientPlayerMovement(WorldPacket&& packet) : ClientPacket(std::move(packet)) { }

            void Read() override;

            MovementInfo movementInfo;
        };

        class MoveUpdate final : public ServerPacket
        {
        public:
            MoveUpdate() : ServerPacket(SMSG_MOVE_UPDATE) { }

            WorldPacket const* Write() override;

            MovementInfo* movementInfo = nullptr;
        };

        struct MonsterSplineFilterKey
        {
            int16 Idx = 0;
            uint16 Speed = 0;
        };

        struct MonsterSplineFilter
        {
            std::vector<MonsterSplineFilterKey> FilterKeys;
            uint8 FilterFlags = 0;
            float BaseSpeed = 0.0f;
            int16 StartOffset = 0;
            float DistToPrevFilterKey = 0.0f;
            int16 AddedToStart = 0;
        };

        struct MonsterSplineSpellEffectExtraData
        {
            ObjectGuid TargetGUID;
            uint32 SpellVisualID = 0;
            uint32 ProgressCurveID = 0;
            uint32 ParabolicCurveID = 0;
        };

        struct MovementSpline
        {
            uint32 Flags = 0;
            uint8 Face = 0;
            uint8 AnimTier = 0;
            uint32 TierTransStartTime = 0;
            int32 Elapsed = 0;
            uint32 MoveTime = 0;
            float JumpGravity = 0.0f;
            uint32 SpecialTime = 0;
            std::vector<TaggedPosition<Position::XYZ>> Points;
            uint8 Mode = 0;
            uint8 VehicleExitVoluntary = 0;
            ObjectGuid TransportGUID;
            int8 VehicleSeat = -1;
            std::vector<TaggedPosition<Position::PackedXYZ>> PackedDeltas;
            Optional<MonsterSplineFilter> SplineFilter;
            Optional<MonsterSplineSpellEffectExtraData> SpellEffectExtraData;
            float FaceDirection = 0.0f;
            ObjectGuid FaceGUID;
            TaggedPosition<Position::XYZ> FaceSpot;
        };

        struct MovementMonsterSpline
        {
            uint32 ID = 0;
            TaggedPosition<Position::XYZ> Destination;
            bool CrzTeleport = false;
            uint8 StopDistanceTolerance = 0;    // Determines how far from spline destination the mover is allowed to stop in place 0, 0, 3.0, 2.76, numeric_limits<float>::max, 1.1, float(INT_MAX); default before this field existed was distance 3.0 (index 2)
            MovementSpline Move;
        };

        class CommonMovement
        {
        public:
            static void WriteCreateObjectSplineDataBlock(::Movement::MoveSpline const& moveSpline, ByteBuffer& data);
        };

        class MonsterMove final : public ServerPacket
        {
        public:
            MonsterMove() : ServerPacket(SMSG_ON_MONSTER_MOVE) { }

            void InitializeSplineData(::Movement::MoveSpline const& moveSpline);

            WorldPacket const* Write() override;

            MovementMonsterSpline SplineData;
            ObjectGuid MoverGUID;
            TaggedPosition<Position::XYZ> Pos;
        };

        class MoveSplineSetSpeed final : public ServerPacket
        {
        public:
            MoveSplineSetSpeed(OpcodeServer opcode) : ServerPacket(opcode, 12) { }

            WorldPacket const* Write() override;

            ObjectGuid MoverGUID;
            float Speed = 1.0f;
        };

        class MoveSetSpeed final : public ServerPacket
        {
        public:
            MoveSetSpeed(OpcodeServer opcode) : ServerPacket(opcode) { }

            WorldPacket const* Write() override;

            ObjectGuid MoverGUID;
            uint32 SequenceIndex = 0;
            float Speed = 1.0f;
        };

        class MoveUpdateSpeed final : public ServerPacket
        {
        public:
            MoveUpdateSpeed(OpcodeServer opcode) : ServerPacket(opcode) { }

            WorldPacket const* Write() override;

            MovementInfo* movementInfo = nullptr;
            float Speed = 1.0f;
        };

        // SMSG_MOVE_SPLINE_ROOT
        // SMSG_MOVE_SPLINE_UNROOT
        // SMSG_MOVE_SPLINE_DISABLE_GRAVITY
        // SMSG_MOVE_SPLINE_ENABLE_GRAVITY
        // SMSG_MOVE_SPLINE_DISABLE_COLLISION
        // SMSG_MOVE_SPLINE_ENABLE_COLLISION
        // SMSG_MOVE_SPLINE_SET_FEATHER_FALL
        // SMSG_MOVE_SPLINE_SET_NORMAL_FALL
        // SMSG_MOVE_SPLINE_SET_HOVER
        // SMSG_MOVE_SPLINE_UNSET_HOVER
        // SMSG_MOVE_SPLINE_SET_WATER_WALK
        // SMSG_MOVE_SPLINE_START_SWIM
        // SMSG_MOVE_SPLINE_STOP_SWIM
        // SMSG_MOVE_SPLINE_SET_RUN_MODE
        // SMSG_MOVE_SPLINE_SET_WALK_MODE
        // SMSG_MOVE_SPLINE_SET_FLYING
        // SMSG_MOVE_SPLINE_UNSET_FLYING
        // SMSG_MOVE_SPLINE_SET_LAND_WALK
        class MoveSplineSetFlag final : public ServerPacket
        {
        public:
            MoveSplineSetFlag(OpcodeServer opcode) : ServerPacket(opcode, 16) { }

            WorldPacket const* Write() override;

            ObjectGuid MoverGUID;
        };

        // SMSG_MOVE_DISABLE_GRAVITY
        // SMSG_MOVE_DISABLE_TRANSITION_BETWEEN_SWIM_AND_FLY
        // SMSG_MOVE_ENABLE_GRAVITY
        // SMSG_MOVE_ENABLE_TRANSITION_BETWEEN_SWIM_AND_FLY
        // SMSG_MOVE_ROOT
        // SMSG_MOVE_SET_CAN_FLY
        // SMSG_MOVE_SET_FEATHER_FALL
        // SMSG_MOVE_SET_HOVERING
        // SMSG_MOVE_SET_IGNORE_MOVEMENT_FORCES
        // SMSG_MOVE_SET_LAND_WALK
        // SMSG_MOVE_SET_NORMAL_FALL
        // SMSG_MOVE_SET_WATER_WALK
        // SMSG_MOVE_UNROOT
        // SMSG_MOVE_UNSET_CAN_FLY
        // SMSG_MOVE_UNSET_IGNORE_MOVEMENT_FORCES
        // SMSG_MOVE_ENABLE_COLLISION
        // SMSG_MOVE_DISABLE_COLLISION
        // SMSG_MOVE_SET_CAN_TURN_WHILE_FALLING
        // SMSG_MOVE_UNSET_HOVERING
        class MoveSetFlag final : public ServerPacket
        {
        public:
            MoveSetFlag(OpcodeServer opcode) : ServerPacket(opcode, 16 + 4) { }

            WorldPacket const* Write() override;

            ObjectGuid MoverGUID;
            uint32 SequenceIndex = 0;
        };

        class TransferPending final : public ServerPacket
        {
        public:
            struct ShipTransferPending
            {
                uint32 ID = 0;
                int32 OriginMapID = -1;
            };

            TransferPending() : ServerPacket(SMSG_TRANSFER_PENDING, 16) { }

            WorldPacket const* Write() override;

            TaggedPosition<Position::XYZ> OldMapPosition;
            Optional<ShipTransferPending> Ship;
            Optional<int32> TransferSpellID;
            int32 MapID = -1;
        };

        class TransferAborted final : public ServerPacket
        {
        public:
            TransferAborted() : ServerPacket(SMSG_TRANSFER_ABORTED, 4 + 1 + 4 + 4) { }

            WorldPacket const* Write() override;

            uint32 TransfertAbort = 0;
            uint8 Arg = 0;
            uint32 MapID = 0;
            int32 MapDifficultyXConditionID = 0;
        };

        enum class NewWorldReason : uint8
        {
            TELEPORT    = 6,    // Name can be little confusing/incorrect
            NORMAL      = 16,   // Normal map change
            SEAMLESS    = 21,   // Teleport to another map without a loading screen, used for outdoor scenarios
            UNK_1       = 23,   // related to bgs?
        };

        class NewWorld final : public ServerPacket
        {
        public:
            NewWorld() : ServerPacket(SMSG_NEW_WORLD, 32) { }

            WorldPacket const* Write() override;

            TaggedPosition<Position::XYZO> Pos;
            TaggedPosition<Position::XYZ> MovementOffset;    // Adjusts all pending movement events by this offset
            int32 MapID = 0;
            NewWorldReason Reason = NewWorldReason::NORMAL;
        };

        class AbortNewWorld final : public ServerPacket
        {
        public:
            AbortNewWorld() : ServerPacket(SMSG_ABORT_NEW_WORLD, 0) { }

            WorldPacket const* Write() override { return &_worldPacket; }
        };

        class WorldPortResponse final : public ClientPacket
        {
        public:
            WorldPortResponse(WorldPacket&& packet) : ClientPacket(CMSG_WORLD_PORT_RESPONSE, std::move(packet)) { }

            void Read() override { }
        };

        struct VehicleTeleport
        {
            uint8 VehicleSeatIndex = 0;
            bool VehicleExitVoluntary = false;
            bool VehicleExitTeleport = false;
        };

        class MoveTeleport final : public ServerPacket
        {
        public:
            MoveTeleport() : ServerPacket(SMSG_MOVE_TELEPORT, 12 + 4 + 16 + 16 + 4 + 1) { }

            WorldPacket const* Write() override;

            TaggedPosition<Position::XYZ> Pos;
            Optional<VehicleTeleport> Vehicle;
            uint32 SequenceIndex = 0;
            ObjectGuid MoverGUID;
            Optional<ObjectGuid> TransportGUID;
            float Facing = 0.0f;
            uint8 PreloadWorld = 0;
        };

        struct MovementForce
        {
            ObjectGuid ID;
            TaggedPosition<Position::XYZ> Direction;
            TaggedPosition<Position::XYZ> TransportPosition;
            uint32 TransportID = 0;
            float Magnitude = 0.0f;
            uint8 Type = 0;
        };

        class MoveUpdateTeleport final : public ServerPacket
        {
        public:
            MoveUpdateTeleport() : ServerPacket(SMSG_MOVE_UPDATE_TELEPORT) { }

            WorldPacket const* Write() override;

            MovementInfo* movementInfo = nullptr;
            std::vector<MovementForce> MovementForces;
            Optional<float> SwimBackSpeed;
            Optional<float> FlightSpeed;
            Optional<float> SwimSpeed;
            Optional<float> WalkSpeed;
            Optional<float> TurnRate;
            Optional<float> RunSpeed;
            Optional<float> FlightBackSpeed;
            Optional<float> RunBackSpeed;
            Optional<float> PitchRate;
        };

        class MoveTeleportAck final : public ClientPacket
        {
        public:
            MoveTeleportAck(WorldPacket&& packet) : ClientPacket(CMSG_MOVE_TELEPORT_ACK, std::move(packet)) { }

            void Read() override;

            ObjectGuid MoverGUID;
            int32 AckIndex = 0;
            int32 ClientMoveTime = 0;
        };

        struct MovementAck
        {
            MovementInfo movementInfo;
            int32 AckIndex = 0;
        };

        class MovementAckMessage final : public ClientPacket
        {
        public:
            MovementAckMessage(WorldPacket&& packet) : ClientPacket(std::move(packet)) { }

            void Read() override;

            MovementAck Ack;
        };

        class MovementSpeedAck final : public ClientPacket
        {
        public:
            MovementSpeedAck(WorldPacket&& packet) : ClientPacket(std::move(packet)) { }

            void Read() override;

            MovementAck Ack;
            float Speed = 0.0f;
        };

        class MoveRemoveMovementForceAck final : public ClientPacket
        {
        public:
            MoveRemoveMovementForceAck(WorldPacket&& packet) : ClientPacket(CMSG_MOVE_REMOVE_MOVEMENT_FORCE_ACK, std::move(packet)) { }

            void Read() override;

            MovementAck Ack;
            ObjectGuid TriggerGUID;
        };

        class MoveApplyMovementForceAck final : public ClientPacket
        {
        public:
            MoveApplyMovementForceAck(WorldPacket&& packet) : ClientPacket(CMSG_MOVE_APPLY_MOVEMENT_FORCE_ACK, std::move(packet)) { }

            void Read() override;

            MovementAck Ack;
            MovementForce MovementForceData;
        };

        class SetActiveMover final : public ClientPacket
        {
        public:
            SetActiveMover(WorldPacket&& packet) : ClientPacket(CMSG_SET_ACTIVE_MOVER, std::move(packet)) { }

            void Read() override;

            ObjectGuid ActiveMover;
        };

        class MoveSetActiveMover final : public ServerPacket
        {
        public:
            MoveSetActiveMover() : ServerPacket(SMSG_MOVE_SET_ACTIVE_MOVER, 8) { }

            WorldPacket const* Write() override;

            ObjectGuid MoverGUID;
        };

        class MoveUpdateKnockBack final : public ServerPacket
        {
        public:
            MoveUpdateKnockBack() : ServerPacket(SMSG_MOVE_UPDATE_KNOCK_BACK) { }

            WorldPacket const* Write() override;

            MovementInfo* movementInfo = nullptr;
        };

        enum UpdateCollisionHeightReason : uint8
        {
            UPDATE_COLLISION_HEIGHT_SCALE = 0,
            UPDATE_COLLISION_HEIGHT_MOUNT = 1,
            UPDATE_COLLISION_HEIGHT_FORCE = 2
        };

        struct CollisionHeightInfo
        {
            float Height = 0.0f;
            float Scale = 0.0f;
            UpdateCollisionHeightReason Reason = UPDATE_COLLISION_HEIGHT_MOUNT;
        };

        class MoveSetCollisionHeight final : public ServerPacket
        {
        public:
            MoveSetCollisionHeight() : ServerPacket(SMSG_MOVE_SET_COLLISION_HEIGHT, 4 + 16 + 4 + 1 + 4 + 4) { }

            WorldPacket const* Write() override;

            ObjectGuid MoverGUID;
            uint32 MountDisplayID = 0;
            uint32 SequenceIndex = 0;
            int32 ScaleDuration = 0;
            CollisionHeightInfo MsgData;
        };

        class MoveUpdateCollisionHeight final : public ServerPacket
        {
        public:
            MoveUpdateCollisionHeight() : ServerPacket(SMSG_MOVE_UPDATE_COLLISION_HEIGHT) { }

            WorldPacket const* Write() override;

            MovementInfo* movementInfo = nullptr;
            float Scale = 1.0f;
            float Height = 1.0f;
        };

        class MoveSetCollisionHeightAck final : public ClientPacket
        {
        public:
            MoveSetCollisionHeightAck(WorldPacket&& packet) : ClientPacket(CMSG_MOVE_SET_COLLISION_HEIGHT_ACK, std::move(packet)) { }

            void Read() override;

            MovementAck Data;
            CollisionHeightInfo MsgData;
            uint32 MountDisplayID = 0;
        };

        class MoveTimeSkipped final : public ClientPacket
        {
        public:
            MoveTimeSkipped(WorldPacket&& packet) : ClientPacket(CMSG_MOVE_TIME_SKIPPED, std::move(packet)) { }

            void Read() override;

            ObjectGuid MoverGUID;
            uint32 TimeSkipped = 0;
        };

        class SummonResponse final : public ClientPacket
        {
        public:
            SummonResponse(WorldPacket&& packet) : ClientPacket(CMSG_SUMMON_RESPONSE, std::move(packet)) { }

            void Read() override;

            bool Accept = false;
            ObjectGuid SummonerGUID;
        };

        class ControlUpdate final : public ServerPacket
        {
        public:
            ControlUpdate() : ServerPacket(SMSG_CONTROL_UPDATE, 16 + 1) { }

            WorldPacket const* Write() override;

            ObjectGuid Guid;
            bool On = false;
        };

        class MoveSplineDone final : public ClientPacket
        {
        public:
            MoveSplineDone(WorldPacket&& packet) : ClientPacket(CMSG_MOVE_SPLINE_DONE, std::move(packet)) { }

            void Read() override;

            MovementInfo movementInfo;
            int32 SplineID = 0;
        };

        class MoveUpdateRemoveMovementForce final : public ServerPacket
        {
        public:
            MoveUpdateRemoveMovementForce() : ServerPacket(SMSG_MOVE_UPDATE_REMOVE_MOVEMENT_FORCE) { }

            WorldPacket const* Write() override;
            
            MovementInfo* movementInfo = nullptr;
            ObjectGuid TriggerGUID;
        };

        class MoveUpdateApplyMovementForce final : public ServerPacket
        {
        public:
            MoveUpdateApplyMovementForce() : ServerPacket(SMSG_MOVE_UPDATE_APPLY_MOVEMENT_FORCE) { }

            WorldPacket const* Write() override;
            
            MovementInfo* movementInfo = nullptr;
            MovementForce MovementForceData;
        };

        struct KnockBackInfo
        {
            TaggedPosition<Position::XY> Direction;
            float HorzSpeed = 0.0f;
            float InitVertSpeed = 0.0f;
        };

        struct MoveStateChange
        {
            MoveStateChange(OpcodeServer messageId, uint32 sequenceIndex) : SequenceIndex(sequenceIndex), MessageID(messageId) { }

            Optional<MovementForce> MovementForceData;
            Optional<KnockBackInfo> KnockBack;
            Optional<CollisionHeightInfo> CollisionHeight;
            Optional<ObjectGuid> Unknown; // MoverGUID ?
            Optional<int32> VehicleRecID;
            Optional<float> Speed;
            uint32 SequenceIndex = 0;
            uint16 MessageID = 0;
        };

        class MoveSetCompoundState final : public ServerPacket
        {
        public:
            MoveSetCompoundState() : ServerPacket(SMSG_MOVE_SET_COMPOUND_STATE, 16 + 4) { }

            WorldPacket const* Write() override;
            
            ObjectGuid MoverGUID;
            std::vector<MoveStateChange> StateChanges;
        };

        class FlightSplineSync final : public ServerPacket
        {
        public:
            FlightSplineSync() : ServerPacket(SMSG_FLIGHT_SPLINE_SYNC, 16 + 4) { }

            WorldPacket const* Write() override;
            
            ObjectGuid Guid;
            float SplineDist = 0.0f;
        };

        class AdjustSplineDuration final : public ServerPacket
        {
        public:
            AdjustSplineDuration() : ServerPacket(SMSG_ADJUST_SPLINE_DURATION, 16 + 4) { }

            WorldPacket const* Write() override;
            
            ObjectGuid Unit;
            float Scale = 0.0f;
        };

        struct MoveKnockBackSpeeds
        {
            float HorzSpeed = 0.0f;
            float VertSpeed = 0.0f;
        };

        class MoveKnockBack final : public ServerPacket
        {
        public:
            MoveKnockBack() : ServerPacket(SMSG_MOVE_KNOCK_BACK, 16 + 8 + 4 + 4 + 4) { }

            WorldPacket const* Write() override;

            ObjectGuid MoverGUID;
            TaggedPosition<Position::XY> Direction;
            MoveKnockBackSpeeds Speeds;
            uint32 SequenceIndex = 0;
        };

        class MoveKnockBackAck final : public ClientPacket
        {
        public:
            MoveKnockBackAck(WorldPacket&& packet) : ClientPacket(CMSG_MOVE_KNOCK_BACK_ACK, std::move(packet)) { }

            void Read() override;

            MovementAck Ack;
            Optional<MoveKnockBackSpeeds> Speeds;
        };

        class MoveApplyMovementForce final : public ServerPacket
        {
        public:
            MoveApplyMovementForce() : ServerPacket(SMSG_MOVE_APPLY_MOVEMENT_FORCE, 67) { }

            WorldPacket const* Write() override;
            
            MovementForce Force;
            ObjectGuid MoverGUID;
            uint32 SequenceIndex = 0;
        };

        class MoveRemoveMovementForce final : public ServerPacket
        {
        public:
            MoveRemoveMovementForce() : ServerPacket(SMSG_MOVE_REMOVE_MOVEMENT_FORCE, 36) { }

            WorldPacket const* Write() override;
            
            ObjectGuid MoverGUID;
            ObjectGuid TriggerGUID;
            uint32 SequenceIndex = 0;
        };

        class TimeSyncResponse final : public ClientPacket
        {
        public:
            TimeSyncResponse(WorldPacket&& packet) : ClientPacket(CMSG_TIME_SYNC_RESPONSE, std::move(packet)) { }

            void Read() override;

            uint32 ClientTime = 0;
            uint32 SequenceIndex = 0;
        };

        class DiscardedTimeSyncAcks final : public ClientPacket
        {
        public:
            DiscardedTimeSyncAcks(WorldPacket&& packet) : ClientPacket(CMSG_DISCARDED_TIME_SYNC_ACKS, std::move(packet)) { }
            
            void Read() override;
            
            uint32 MaxSequenceIndex = 0;
        };

        class TimeSyncResponseDropped final : public ClientPacket
        {
        public:
            TimeSyncResponseDropped(WorldPacket&& packet) : ClientPacket(CMSG_TIME_SYNC_RESPONSE_DROPPED, std::move(packet)) { }
            
            void Read() override;
            
            uint32 SequenceIndexFirst = 0;
            uint32 SequenceIndexLast = 0;
        };

        class TimeSyncResponseFailed final : public ClientPacket
        {
        public:
            TimeSyncResponseFailed(WorldPacket&& packet) : ClientPacket(CMSG_TIME_SYNC_RESPONSE_FAILED, std::move(packet)) { }
            
            void Read() override;
            
            uint32 SequenceIndex = 0;
        };

        class TimeSyncRequest final : public ServerPacket
        {
        public:
            TimeSyncRequest() : ServerPacket(SMSG_TIME_SYNC_REQUEST, 4) { }

            WorldPacket const* Write() override;

            uint32 SequenceIndex = 0;
        };

        class SuspendToken final : public ServerPacket
        {
        public:
            SuspendToken() : ServerPacket(SMSG_SUSPEND_TOKEN, 4 + 1) { }

            WorldPacket const* Write() override;

            uint32 SequenceIndex = 1;
            uint32 Reason = 1;
        };

        class SuspendTokenResponse final : public ClientPacket
        {
        public:
            SuspendTokenResponse(WorldPacket&& packet) : ClientPacket(CMSG_SUSPEND_TOKEN_RESPONSE, std::move(packet)) { }

            void Read() override;

            uint32 SequenceIndex = 0;
        };

        class ResumeToken final : public ServerPacket
        {
        public:
            ResumeToken() : ServerPacket(SMSG_RESUME_TOKEN, 4 + 1) { }

            WorldPacket const* Write() override;

            uint32 SequenceIndex = 1;
            uint32 Reason = 1;
        };

        class MoveSkipTime final : public ServerPacket
        {
        public:
            MoveSkipTime() : ServerPacket(SMSG_MOVE_SKIP_TIME, 16 + 4) { }

            WorldPacket const* Write() override;

            ObjectGuid MoverGUID;
            uint32 SkippedTime = 0;
        };
    }
    ByteBuffer& operator<<(ByteBuffer& data, Movement::MonsterSplineFilterKey const& monsterSplineFilterKey);
    ByteBuffer& operator<<(ByteBuffer& data, Movement::MonsterSplineFilter const& monsterSplineFilter);
    ByteBuffer& operator<<(ByteBuffer& data, Movement::MovementSpline const& movementSpline);
    ByteBuffer& operator<<(ByteBuffer& data, Movement::MovementMonsterSpline const& movementMonsterSpline);
}

ByteBuffer& operator>>(ByteBuffer& data, MovementInfo& movementInfo);
ByteBuffer& operator<<(ByteBuffer& data, MovementInfo& movementInfo);

ByteBuffer& operator>>(ByteBuffer& data, MovementInfo::TransportInfo& transportInfo);
ByteBuffer& operator<<(ByteBuffer& data, MovementInfo::TransportInfo const& transportInfo);
ByteBuffer& operator>>(ByteBuffer& data, WorldPackets::Movement::MovementAck& movementAck);
ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Movement::MovementForce const& movementForce);

#endif // MovementPackets_h__
